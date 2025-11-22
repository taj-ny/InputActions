/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2025 Marcin Woźniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "StandaloneInputBackend.h"
#include <QDir>
#include <QSocketNotifier>
#include <fcntl.h>
#include <linux/uinput.h>
#include <sys/inotify.h>

using namespace InputActions;

static constexpr uint32_t MAX_INITIALIZATION_ATTEMPTS = 5;

StandaloneInputBackend::StandaloneInputBackend()
{
    m_libinputBlockingInterface = {
        .open_restricted = &StandaloneInputBackend::openRestrictedGrab,
        .close_restricted = &StandaloneInputBackend::closeRestricted,
    };
    m_libinputNonBlockingInterface = {
        .open_restricted = &StandaloneInputBackend::openRestricted,
        .close_restricted = &StandaloneInputBackend::closeRestricted,
    };

    m_inotifyFd = inotify_init();
    fcntl(m_inotifyFd, F_SETFD, FD_CLOEXEC);
    fcntl(m_inotifyFd, F_SETFL, fcntl(m_inotifyFd, F_GETFL, 0) | O_NONBLOCK);

    m_inotifyNotifier = std::make_unique<QSocketNotifier>(m_inotifyFd, QSocketNotifier::Read);
    m_inotifyNotifier->setEnabled(false);
    connect(m_inotifyNotifier.get(), &QSocketNotifier::activated, this, &StandaloneInputBackend::inotifyTimerTick);
    inotify_add_watch(m_inotifyFd, "/dev/input", IN_CREATE | IN_DELETE);

    connect(&m_deviceInitializationRetryTimer, &QTimer::timeout, this, &StandaloneInputBackend::deviceInitializationRetryTimerTick);
    m_deviceInitializationRetryTimer.setInterval(1000);
}

StandaloneInputBackend::~StandaloneInputBackend()
{
    if (m_inotifyFd != -1) {
        close(m_inotifyFd);
    }
    reset();
}

void StandaloneInputBackend::initialize()
{
    LibinputInputBackend::initialize();

    for (const auto &entry : QDir("/dev/input").entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::System)) {
        evdevDeviceAdded(entry.filePath());
    }

    m_inotifyNotifier->setEnabled(true);
    m_deviceInitializationRetryTimer.start();
}

void StandaloneInputBackend::reset()
{
    for (const auto &[device, data] : m_devices) {
        if (device->properties().grab()) {
            resetDevice(device.get(), data.get());
        }
        deviceRemoved(device.get());
    }
    m_devices.clear();
    m_inotifyNotifier->setEnabled(false);
    m_deviceInitializationRetryTimer.stop();
    LibinputInputBackend::reset();
}

void StandaloneInputBackend::evdevDeviceAdded(const QString &path)
{
    if (!path.startsWith("/dev/input/event")) {
        return;
    }

    if (!tryAddEvdevDevice(path)) {
        m_deviceInitializationQueue[path] = 0;
    }
}

bool StandaloneInputBackend::tryAddEvdevDevice(const QString &path)
{
    for (const auto &[device, data] : m_devices) {
        if (path == data->libinputEventInjectionDevicePath || path == data->outputDevicePath) {
            return true;
        }
    }

    auto data = std::make_unique<ExtraDeviceData>();
    data->path = path.toStdString();
    data->libinput = libinput_path_create_context(&m_libinputNonBlockingInterface, nullptr);
    data->libinputDevice = libinput_path_add_device(data->libinput, data->path.c_str());
    if (!data->libinputDevice) {
        // May fail if opened before the udev rule sets ACLs, initialization will be attempted again later.
        return errno == ENOENT;
    }
    libinput_device_ref(data->libinputDevice);

    const QString name(libinput_device_get_name(data->libinputDevice));
    const QString sysName(libinput_device_get_sysname(data->libinputDevice));
    InputDeviceType deviceType{};
    auto udevDevice = libinput_device_get_udev_device(data->libinputDevice);
    if (udev_device_get_property_value(udevDevice, "ID_INPUT_MOUSE")) {
        deviceType = InputDeviceType::Mouse;
    } else if (udev_device_get_property_value(udevDevice, "ID_INPUT_KEYBOARD")) {
        deviceType = InputDeviceType::Keyboard;
    } else if (udev_device_get_property_value(udevDevice, "ID_INPUT_TOUCHPAD")) {
        deviceType = InputDeviceType::Touchpad;
    } else {
        return true;
    }
    auto device = std::make_unique<InputDevice>(deviceType, name, sysName);
    const auto properties = deviceProperties(device.get());

    if (properties.ignore()) {
        return true;
    }

    if (properties.grab()) {
        const auto fd = open(data->path.c_str(), O_RDONLY | O_NONBLOCK);
        fcntl(fd, F_SETFD, FD_CLOEXEC);
        libevdev_new_from_fd(fd, &data->libevdev);
        if (!isDeviceNeutral(device.get(), data.get())) {
            qWarning(INPUTACTIONS).noquote().nospace()
                << QString("Failed to initialize device '%1': device is not in a neutral state and cannot be grabbed").arg(device->name());
            return false;
        }
        ioctl(fd, EVIOCGRAB, 1);

        data->libevdevNotifier = std::make_unique<QSocketNotifier>(static_cast<qintptr>(libevdev_get_fd(data->libevdev)), QSocketNotifier::Read);
        connect(data->libevdevNotifier.get(), &QSocketNotifier::activated, this, &StandaloneInputBackend::poll);

        // Create new context that uses openRestrictedGrab
        libinput_unref(data->libinput);
        data->libinputDevice = {};
        data->libinput = libinput_path_create_context(&m_libinputBlockingInterface, nullptr);

        auto virtualDeviceName = name.toStdString() + " (InputActions internal)";
        libevdev_set_name(data->libevdev, virtualDeviceName.c_str());
        if (const auto error = libevdev_uinput_create_from_device(data->libevdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &data->libinputEventInjectionDevice)) {
            qWarning(INPUTACTIONS).noquote().nospace()
                << QString("Failed to create virtual device '%1' (errno %2)").arg(QString::fromStdString(virtualDeviceName)).arg(QString::number(-error));
            return false;
        }

        fcntl(libevdev_uinput_get_fd(data->libinputEventInjectionDevice),
              F_SETFL,
              fcntl(libevdev_uinput_get_fd(data->libinputEventInjectionDevice), F_GETFL, 0) & ~O_NONBLOCK);
        data->libinputEventInjectionDevicePath = libevdev_uinput_get_devnode(data->libinputEventInjectionDevice);
        data->libinputDevice = libinput_path_add_device(data->libinput, data->libinputEventInjectionDevicePath.c_str());
        if (data->libinputDevice) {
            // Will almost always fail due to being opened before the udev rule sets ACLs, initialization will be attempted again later.
            libinput_device_ref(data->libinputDevice);
        }

        virtualDeviceName = name.toStdString() + " (InputActions output)";
        libevdev_set_name(data->libevdev, virtualDeviceName.c_str());
        if (const auto error = libevdev_uinput_create_from_device(data->libevdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &data->outputDevice)) {
            qWarning(INPUTACTIONS).noquote().nospace()
                << QString("Failed to create virtual device '%1' (errno %2)").arg(QString::fromStdString(virtualDeviceName)).arg(QString::number(-error));
            return false;
        }

        data->outputDevicePath = libevdev_uinput_get_devnode(data->outputDevice);

        libevdev_set_name(data->libevdev, name.toStdString().c_str());

        if (deviceType == InputDeviceType::Touchpad) {
            if (properties.handleLibevdevEvents()) {
                LibevdevComplementaryInputBackend::addDevice(device.get(), data->libevdev, false);
            }

            connect(&data->touchpadStateResetTimer, &QTimer::timeout, this, [this, device = device.get(), data = data.get()]() {
                resetDevice(device, data);
            });
        }
    } else {
        if (properties.handleLibevdevEvents()) {
            LibevdevComplementaryInputBackend::deviceAdded(device.get());
        }
    }

    if (data->libinputDevice) {
        finishLibinputDeviceInitialization(device.get(), data.get());
    }

    data->libinputNotifier = std::make_unique<QSocketNotifier>(static_cast<qintptr>(libinput_get_fd(data->libinput)), QSocketNotifier::Read);
    connect(data->libinputNotifier.get(), &QSocketNotifier::activated, this, &StandaloneInputBackend::poll);

    InputBackend::deviceAdded(device.get());
    m_devices.emplace_back(std::move(device), std::move(data));
    return true;
}

void StandaloneInputBackend::finishLibinputDeviceInitialization(InputDevice *device, ExtraDeviceData *data)
{
    if (device->type() == InputDeviceType::Touchpad) {
        libinput_device_config_tap_set_enabled(data->libinputDevice, LIBINPUT_CONFIG_TAP_ENABLED);
    }

    libinput_dispatch(data->libinput);
    while (auto *libinputEvent = libinput_get_event(data->libinput)) {
        libinput_event_destroy(libinputEvent);
    }
}

void StandaloneInputBackend::evdevDeviceRemoved(const QString &path)
{
    for (auto it = m_devices.begin(); it != m_devices.end(); ++it) {
        if (it->second->path == path) {
            deviceRemoved(it->first.get());
            m_devices.erase(it);
            break;
        }
    }
}

void StandaloneInputBackend::inotifyTimerTick()
{
    std::array<int8_t, 16 * (sizeof(inotify_event) + NAME_MAX + 1)> buffer{};
    while (true) {
        auto length = read(m_inotifyFd, &buffer, sizeof(buffer));
        if (length <= 0) {
            break;
        }

        for (int i = 0; i < length;) {
            auto *event = (inotify_event *)&buffer[i];
            const auto path = QString("/dev/input/%1").arg(QString::fromLocal8Bit(event->name, event->len).replace('\0', ""));

            if (event->mask & IN_CREATE) {
                evdevDeviceAdded(path);
            } else if (event->mask & IN_DELETE) {
                evdevDeviceRemoved(path);
            }
            i += sizeof(inotify_event) + event->len;
        }
    }
}

void StandaloneInputBackend::deviceInitializationRetryTimerTick()
{
    for (auto it = m_deviceInitializationQueue.begin(); it != m_deviceInitializationQueue.end();) {
        const auto &path = it->first;
        auto &attempts = it->second;

        if (tryAddEvdevDevice(path) || ++attempts == MAX_INITIALIZATION_ATTEMPTS) {
            it = m_deviceInitializationQueue.erase(it);
            continue;
        }

        ++it;
    }

    for (auto it = m_devices.begin(); it != m_devices.end();) {
        const auto &device = it->first;
        const auto &data = it->second;

        if (!data->libinputDevice) {
            data->libinputDevice = libinput_path_add_device(data->libinput, data->libinputEventInjectionDevicePath.c_str());
            if (data->libinputDevice) {
                libinput_device_ref(data->libinputDevice);
                finishLibinputDeviceInitialization(device.get(), data.get());
            } else {
                if (++data->initializationAttempts == MAX_INITIALIZATION_ATTEMPTS) {
                    it = m_devices.erase(it);
                    continue;
                }
            }
        }

        ++it;
    }
}

bool StandaloneInputBackend::handleEvent(InputDevice *sender, libinput_event *event)
{
    const auto type = libinput_event_get_type(event);
    switch (type) {
        case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
        case LIBINPUT_EVENT_GESTURE_HOLD_END:
        case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
        case LIBINPUT_EVENT_GESTURE_PINCH_END:
        case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
        case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
        case LIBINPUT_EVENT_GESTURE_SWIPE_END:
        case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE: {
            auto *gestureEvent = libinput_event_get_gesture_event(event);
            const auto fingers = libinput_event_gesture_get_finger_count(gestureEvent);

            auto cancelled = false;
            switch (type) {
                case LIBINPUT_EVENT_GESTURE_HOLD_END:
                case LIBINPUT_EVENT_GESTURE_PINCH_END:
                case LIBINPUT_EVENT_GESTURE_SWIPE_END:
                    cancelled = libinput_event_gesture_get_cancelled(gestureEvent) == 1;
                    break;
            }

            switch (type) {
                case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
                    return touchpadHoldBegin(sender, fingers);
                case LIBINPUT_EVENT_GESTURE_HOLD_END:
                    return touchpadHoldEnd(sender, cancelled);
                case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
                    touchpadPinchBegin(sender, fingers);
                    // Resetting the touchpad state one frame before libinput recognizes a pinch gesture messes up the state machine, probably a libinput bug.
                    // Resetting it one frame before a pinch update event does not trigger the bug, and the gesture is still blocked, since clients and
                    // compositors only start executing actions after the first update event.
                    return false;
                case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE: {
                    const auto scale = libinput_event_gesture_get_scale(gestureEvent);
                    const auto angleDelta = libinput_event_gesture_get_angle_delta(gestureEvent);
                    return touchpadPinchUpdate(sender, scale, angleDelta);
                }
                case LIBINPUT_EVENT_GESTURE_PINCH_END:
                    return touchpadPinchEnd(sender, cancelled);
                case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
                    return touchpadSwipeBegin(sender, fingers);
                case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE: {
                    const QPointF acceleratedDelta(libinput_event_gesture_get_dx(gestureEvent), libinput_event_gesture_get_dy(gestureEvent));
                    const QPointF unacceleratedDelta(libinput_event_gesture_get_dx_unaccelerated(gestureEvent),
                                                     libinput_event_gesture_get_dy_unaccelerated(gestureEvent));
                    return touchpadSwipeUpdate(sender, {acceleratedDelta, unacceleratedDelta});
                }
                case LIBINPUT_EVENT_GESTURE_SWIPE_END:
                    return touchpadSwipeEnd(sender, cancelled);
            }
            break;
        }
        case LIBINPUT_EVENT_KEYBOARD_KEY: {
            auto *keyboardEvent = libinput_event_get_keyboard_event(event);
            const auto key = libinput_event_keyboard_get_key(keyboardEvent);
            const auto state = libinput_event_keyboard_get_key_state(keyboardEvent) == LIBINPUT_KEY_STATE_PRESSED;

            sender->setKeyState(key, state);
            return keyboardKey(sender,
                               libinput_event_keyboard_get_key(keyboardEvent),
                               libinput_event_keyboard_get_key_state(keyboardEvent) == LIBINPUT_KEY_STATE_PRESSED);
        }
        case LIBINPUT_EVENT_POINTER_AXIS:
        case LIBINPUT_EVENT_POINTER_BUTTON:
        case LIBINPUT_EVENT_POINTER_MOTION:
            auto *pointerEvent = libinput_event_get_pointer_event(event);
            switch (type) {
                case LIBINPUT_EVENT_POINTER_AXIS: {
                    static const auto axis = [](auto *pointerEvent, auto axis) {
                        return libinput_event_pointer_has_axis(pointerEvent, axis) ? libinput_event_pointer_get_axis_value(pointerEvent, axis) : 0;
                    };
                    const QPointF delta(axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL), axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
                    return pointerAxis(sender, delta);
                }
                case LIBINPUT_EVENT_POINTER_BUTTON: {
                    const auto button = libinput_event_pointer_get_button(pointerEvent);
                    const auto state = libinput_event_pointer_get_button_state(pointerEvent);
                    return pointerButton(sender, scanCodeToMouseButton(button), button, state == LIBINPUT_BUTTON_STATE_PRESSED);
                }
                case LIBINPUT_EVENT_POINTER_MOTION:
                    const QPointF acceleratedDelta(libinput_event_pointer_get_dx(pointerEvent), libinput_event_pointer_get_dy(pointerEvent));
                    const QPointF unacceleratedDelta(libinput_event_pointer_get_dx_unaccelerated(pointerEvent),
                                                     libinput_event_pointer_get_dy_unaccelerated(pointerEvent));
                    return pointerMotion(sender, {acceleratedDelta, unacceleratedDelta});
            }
    }
    return false;
}

LibinputEventsProcessingResult StandaloneInputBackend::handleLibinputEvents(InputDevice *device, libinput *libinput)
{
    libinput_dispatch(libinput);

    // FIXME: One evdev frame can result in multiple libinput events, but one blocked libinput event will block the entire evdev frame.
    LibinputEventsProcessingResult result;
    while (auto *libinputEvent = libinput_get_event(libinput)) {
        result.block = handleEvent(device, libinputEvent) || result.block;
        libinput_event_destroy(libinputEvent);
        result.eventCount++;
    }
    return result;
}

bool StandaloneInputBackend::isDeviceNeutral(const InputDevice *device, const ExtraDeviceData *data)
{
    switch (device->type()) {
        case InputDeviceType::Keyboard:
        case InputDeviceType::Mouse:
            for (int code = 0; code < KEY_MAX; code++) {
                if (libevdev_has_event_code(data->libevdev, EV_KEY, code) && libevdev_get_event_value(data->libevdev, EV_KEY, code)) {
                    return false;
                }
            }
            break;
        case InputDeviceType::Touchpad:
            return !libevdev_has_event_code(data->libevdev, EV_KEY, BTN_TOUCH) || !libevdev_get_event_value(data->libevdev, EV_KEY, BTN_TOUCH);
    }
    return true;
}

void StandaloneInputBackend::resetDevice(const InputDevice *device, const ExtraDeviceData *data)
{
    switch (device->type()) {
        case InputDeviceType::Keyboard:
        case InputDeviceType::Mouse: {
            bool syn{};
            for (int code = 0; code < KEY_MAX; code++) {
                if (libevdev_has_event_code(data->libevdev, EV_KEY, code) && libevdev_get_event_value(data->libevdev, EV_KEY, code)) {
                    syn = true;
                    libevdev_uinput_write_event(data->outputDevice, EV_KEY, code, 0);
                }
            }

            if (syn) {
                libevdev_uinput_write_event(data->outputDevice, EV_SYN, SYN_REPORT, 0);
            }
            break;
        }
        case InputDeviceType::Touchpad:
            // Reverse order so that ABS_MT_SLOT is equal to 0 after
            for (int i = device->m_touchPoints.size() - 1; i >= 0; i--) {
                libevdev_uinput_write_event(data->outputDevice, EV_ABS, ABS_MT_SLOT, i);
                libevdev_uinput_write_event(data->outputDevice, EV_ABS, ABS_MT_TRACKING_ID, -1);
            }

            libevdev_uinput_write_event(data->outputDevice, EV_KEY, BTN_TOOL_QUINTTAP, 0);
            libevdev_uinput_write_event(data->outputDevice, EV_KEY, BTN_TOOL_QUADTAP, 0);
            libevdev_uinput_write_event(data->outputDevice, EV_KEY, BTN_TOOL_TRIPLETAP, 0);
            libevdev_uinput_write_event(data->outputDevice, EV_KEY, BTN_TOUCH, 0);
            libevdev_uinput_write_event(data->outputDevice, EV_KEY, BTN_TOOL_DOUBLETAP, 0);
            libevdev_uinput_write_event(data->outputDevice, EV_KEY, BTN_TOOL_FINGER, 0);
            libevdev_uinput_write_event(data->outputDevice, EV_ABS, ABS_PRESSURE, 0);
            libevdev_uinput_write_event(data->outputDevice, EV_SYN, SYN_REPORT, 0);
            break;
    }
}

void StandaloneInputBackend::copyTouchpadState(const ExtraDeviceData *data) const
{
    for (int code = 0; code < KEY_MAX; code++) {
        if (!libevdev_has_event_code(data->libevdev, EV_KEY, code)) {
            continue;
        }

        libevdev_uinput_write_event(data->outputDevice, EV_KEY, code, libevdev_get_event_value(data->libevdev, EV_KEY, code));
    }

    for (int code = 0; code < ABS_MAX; code++) {
        if ((code >= ABS_MT_SLOT && code <= ABS_MT_TOOL_Y) || !libevdev_has_event_code(data->libevdev, EV_ABS, code)) {
            continue;
        }

        const auto *info = libevdev_get_abs_info(data->libevdev, code);
        libevdev_uinput_write_event(data->outputDevice, EV_ABS, code, info->value);
    }

    for (int slot = 0; slot <= libevdev_get_num_slots(data->libevdev); slot++) {
        libevdev_uinput_write_event(data->outputDevice, EV_ABS, ABS_MT_SLOT, slot);

        for (int code = ABS_MT_SLOT; code <= ABS_MT_TOOL_Y; code++) {
            if (code == ABS_MT_SLOT || !libevdev_has_event_code(data->libevdev, EV_ABS, code)) {
                continue;
            }

            libevdev_uinput_write_event(data->outputDevice, EV_ABS, code, libevdev_get_slot_value(data->libevdev, slot, code));
        }
    }

    libevdev_uinput_write_event(data->outputDevice, EV_ABS, ABS_MT_SLOT, libevdev_get_current_slot(data->libevdev));
    libevdev_uinput_write_event(data->outputDevice, EV_SYN, SYN_REPORT, 0);
}

void StandaloneInputBackend::poll()
{
    LibevdevComplementaryInputBackend::poll();

    std::vector<input_event> frame;
    for (auto &[device, data] : m_devices) {
        if (!device->properties().grab()) {
            handleLibinputEvents(device.get(), data->libinput);
            continue;
        }

        int status{};
        while (true) {
            input_event evdevEvent{};
            auto flags = status == LIBEVDEV_READ_STATUS_SYNC ? LIBEVDEV_READ_FLAG_SYNC : LIBEVDEV_READ_FLAG_NORMAL;
            status = libevdev_next_event(data->libevdev, flags, &evdevEvent);
            if (status != LIBEVDEV_READ_STATUS_SUCCESS && status != LIBEVDEV_READ_STATUS_SYNC) {
                // Handle events generated after a delay, e.g. pointer button after tapping
                handleLibinputEvents(device.get(), data->libinput);
                break;
            }

            frame.push_back(evdevEvent);
            if (device->properties().handleLibevdevEvents()) {
                LibevdevComplementaryInputBackend::handleEvdevEvent(device.get(), evdevEvent);
            }

            if (evdevEvent.type != EV_SYN) {
                continue;
            }

            for (const auto &event : frame) {
                libevdev_uinput_write_event(data->libinputEventInjectionDevice, event.type, event.code, event.value);
            }
            const auto result = handleLibinputEvents(device.get(), data->libinput);

            // Copy state of the real device to the output device if events suddenly stop being blocked while the device is not in a neutral state
            if (data->touchpadBlocked && !result.block && result.eventCount) {
                data->touchpadStateResetTimer.stop();
                data->touchpadBlocked = false;
                copyTouchpadState(data.get());
            }

            // Touchpad gestures are blocked by blocking the current and all next frames until all fingers are lifted, and changing the state of the output
            // device to neutral after 200ms. The delay is required to block tap gestures, but doesn't affect motion gesture blocking negatively.
            if (result.block && device->type() == InputDeviceType::Touchpad && !data->touchpadBlocked) {
                data->touchpadBlocked = true;
                data->touchpadStateResetTimer.start(200);
            } else if (data->touchpadNeutral && data->touchpadStateResetTimer.isActive()) {
                data->touchpadStateResetTimer.stop();
                resetDevice(device.get(), data.get());
            }

            if (!result.block && !data->touchpadBlocked) {
                for (const auto &event : frame) {
                    libevdev_uinput_write_event(data->outputDevice, event.type, event.code, event.value);
                }
            }
            frame.clear();

            data->touchpadNeutral = false;
        }

        if (device->validTouchPoints().empty()) {
            data->touchpadNeutral = true;
            data->touchpadBlocked = false;
        }
    }
}

libevdev_uinput *StandaloneInputBackend::outputDevice(const InputActions::InputDevice *device) const
{
    for (const auto &[libinputactionsDevice, data] : m_devices) {
        if (libinputactionsDevice.get() == device) {
            return data->outputDevice;
        }
    }
    return nullptr;
}

int StandaloneInputBackend::openRestricted(const char *path, int flags, void *data)
{
    int fd = open(path, flags);
    return fd < 0 ? -errno : fd;
}

int StandaloneInputBackend::openRestrictedGrab(const char *path, int flags, void *data)
{
    int fd = open(path, flags);
    ioctl(fd, EVIOCGRAB, 1);
    return fd < 0 ? -errno : fd;
}

void StandaloneInputBackend::closeRestricted(int fd, void *data)
{
    close(fd);
}

StandaloneInputBackend::ExtraDeviceData::~ExtraDeviceData()
{
    if (libevdev) {
        close(libevdev_get_fd(libevdev));
        libevdev_free(libevdev);
    }
    if (libinputDevice) {
        libinput_device_unref(libinputDevice);
    }
    if (libinput) {
        libinput_unref(libinput);
    }
    if (libinputEventInjectionDevice) {
        libevdev_uinput_destroy(libinputEventInjectionDevice);
    }
    if (outputDevice) {
        libevdev_uinput_destroy(outputDevice);
    }
}