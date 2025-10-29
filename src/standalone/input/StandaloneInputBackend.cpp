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
#include <fcntl.h>
#include <linux/uinput.h>
#include <poll.h>
#include <ranges>
#include <sys/inotify.h>

using namespace InputActions;

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
    inotify_add_watch(m_inotifyFd, "/dev/input", IN_CREATE | IN_DELETE);

    connect(&m_inotifyTimer, &QTimer::timeout, this, &StandaloneInputBackend::inotifyRead);
    m_inotifyTimer.setInterval(1000);
    m_inotifyTimer.start();
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

    usleep(500000); // Wait 500ms to reduce the possibility of the device not being in a neutral state
    for (const auto &entry : QDir("/dev/input").entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::System)) {
        evdevDeviceAdded(entry.filePath());
    }
}

void StandaloneInputBackend::reset()
{
    for (const auto &[device, _] : m_devices) {
        deviceRemoved(device.get());
    }
    m_devices.clear();
    LibinputInputBackend::reset();
}

void StandaloneInputBackend::evdevDeviceAdded(const QString &path)
{
    if (!path.contains("event")) {
        return;
    }
    for (const auto &[device, data] : m_devices) {
        if (path == data->libinputEventInjectionDevicePath || path == data->outputDevicePath) {
            return;
        }
    }

    auto data = std::make_unique<ExtraDeviceData>();
    data->path = path.toStdString();
    data->libinput = libinput_path_create_context(&m_libinputNonBlockingInterface, nullptr);
    data->libinputDevice = libinput_path_add_device(data->libinput, data->path.c_str());
    if (!data->libinputDevice) {
        return;
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
        return;
    }
    auto device = std::make_unique<InputDevice>(deviceType, name, sysName);
    const auto properties = deviceProperties(device.get());

    if (properties.ignore()) {
        return;
    }

    if (properties.grab()) {
        const auto fd = open(data->path.c_str(), O_RDONLY | O_NONBLOCK);
        ioctl(fd, EVIOCGRAB, 1);
        fcntl(fd, F_SETFD, FD_CLOEXEC);
        libevdev_new_from_fd(fd, &data->libevdev);

        // Create new context that uses openRestrictedGrab
        libinput_unref(data->libinput);
        data->libinputDevice = {};
        data->libinput = libinput_path_create_context(&m_libinputBlockingInterface, nullptr);

        auto virtualDeviceName = name.toStdString() + " (InputActions internal 1)";
        libevdev_set_name(data->libevdev, virtualDeviceName.c_str());
        libevdev_uinput_create_from_device(data->libevdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &data->libinputEventInjectionDevice);
        fcntl(libevdev_uinput_get_fd(data->libinputEventInjectionDevice),
              F_SETFL,
              fcntl(libevdev_uinput_get_fd(data->libinputEventInjectionDevice), F_GETFL, 0) & ~O_NONBLOCK);
        data->libinputEventInjectionDevicePath = libevdev_uinput_get_devnode(data->libinputEventInjectionDevice);
        data->libinputDevice = libinput_path_add_device(data->libinput, data->libinputEventInjectionDevicePath.c_str());
        libinput_device_ref(data->libinputDevice);

        virtualDeviceName = name.toStdString() + " (InputActions output)";
        libevdev_set_name(data->libevdev, virtualDeviceName.c_str());
        libevdev_uinput_create_from_device(data->libevdev, LIBEVDEV_UINPUT_OPEN_MANAGED, &data->outputDevice);
        data->outputDevicePath = libevdev_uinput_get_devnode(data->outputDevice);

        libevdev_set_name(data->libevdev, name.toStdString().c_str());

        if (deviceType == InputDeviceType::Touchpad) {
            LibevdevComplementaryInputBackend::addDevice(device.get(), data->libevdev, false);

            connect(&data->touchpadStateResetTimer, &QTimer::timeout, this, [device = device.get(), data = data.get()]() {
                resetTouchpad(device, data);
            });
        }
    } else {
        LibevdevComplementaryInputBackend::deviceAdded(device.get());
    }

    if (deviceType == InputDeviceType::Touchpad) {
        libinput_device_config_tap_set_enabled(data->libinputDevice, LIBINPUT_CONFIG_TAP_ENABLED);
    }

    libinput_dispatch(data->libinput);
    while (auto *libinputEvent = libinput_get_event(data->libinput)) {
        libinput_event_destroy(libinputEvent);
    }

    InputBackend::deviceAdded(device.get());

    m_devices.emplace_back(std::move(device), std::move(data));
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

void StandaloneInputBackend::waitForEvents(int timeout)
{
    std::vector<pollfd> pollfds(m_devices.size());
    for (const auto &[_, data] : m_devices) {
        if (data->libevdev) {
            pollfds.push_back({
                .fd = libevdev_get_fd(data->libevdev),
                .events = POLLIN,
            });
        }
        pollfds.push_back({
            .fd = libinput_get_fd(data->libinput),
            .events = POLLIN,
        });
    }

    ::poll(pollfds.data(), pollfds.size(), timeout);
}

void StandaloneInputBackend::inotifyRead()
{
    std::array<int8_t, 16 * (sizeof(inotify_event) + NAME_MAX + 1)> buffer{};
    auto length = read(m_inotifyFd, &buffer, sizeof(buffer));
    for (int i = 0; i < length;) {
        auto *event = (inotify_event *)(&buffer[i]);
        const auto path = QString("/dev/input/%1").arg(QString::fromLocal8Bit(event->name, event->len).replace('\0', ""));
        if (event->mask & IN_CREATE) {
            evdevDeviceAdded(path);
        } else if (event->mask & IN_DELETE) {
            evdevDeviceRemoved(path);
        }
        i += sizeof(inotify_event) + event->len;
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
                    const QPointF delta(libinput_event_gesture_get_dx_unaccelerated(gestureEvent), libinput_event_gesture_get_dy_unaccelerated(gestureEvent));
                    return touchpadSwipeUpdate(sender, delta);
                }
                case LIBINPUT_EVENT_GESTURE_SWIPE_END:
                    return touchpadSwipeEnd(sender, cancelled);
            }
            break;
        }
        case LIBINPUT_EVENT_KEYBOARD_KEY: {
            auto *keyboardEvent = libinput_event_get_keyboard_event(event);
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
                    return pointerMotion(sender, {libinput_event_pointer_get_dx(pointerEvent), libinput_event_pointer_get_dy(pointerEvent)});
            }
    }
    return false;
}

bool StandaloneInputBackend::handleLibinputEvents(InputDevice *device, libinput *libinput)
{
    libinput_dispatch(libinput);

    // FIXME: One evdev frame can result in multiple libinput events, but one blocked libinput event will block the entire evdev frame.
    bool block{};
    while (auto *libinputEvent = libinput_get_event(libinput)) {
        block = handleEvent(device, libinputEvent) || block;
        libinput_event_destroy(libinputEvent);
    }
    return block;
}

void StandaloneInputBackend::resetTouchpad(const InputDevice *device, const ExtraDeviceData *data)
{
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
            LibevdevComplementaryInputBackend::handleEvdevEvent(device.get(), evdevEvent);

            if (evdevEvent.type != EV_SYN) {
                continue;
            }

            for (const auto &event : frame) {
                libevdev_uinput_write_event(data->libinputEventInjectionDevice, event.type, event.code, event.value);
            }
            const auto block = handleLibinputEvents(device.get(), data->libinput);

            // Touchpad gestures are blocked by blocking the current and all next frames until all fingers are lifted, and changing the state of the output
            // device to neutral after 200ms. The delay is required to block tap gestures, but doesn't affect motion gesture blocking negatively.
            if (block && device->type() == InputDeviceType::Touchpad && !data->touchpadBlocked) {
                data->touchpadBlocked = true;
                data->touchpadStateResetTimer.start(200);
            } else if (data->touchpadNeutral && data->touchpadStateResetTimer.isActive()) {
                data->touchpadStateResetTimer.stop();
                resetTouchpad(device.get(), data.get());
            }

            if (!block && !data->touchpadBlocked) {
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