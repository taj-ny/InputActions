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
#include "interfaces/EvdevInputEmitter.h"
#include <QDir>
#include <QSocketNotifier>
#include <fcntl.h>
#include <libevdev-cpp/LibevdevDevice.h>
#include <libevdev-cpp/LibevdevUinputDevice.h>
#include <libinput-cpp/LibinputDevice.h>
#include <libinput-cpp/LibinputEvent.h>
#include <libinput-cpp/LibinputGestureEvent.h>
#include <libinput-cpp/LibinputKeyboardEvent.h>
#include <libinput-cpp/LibinputPointerEvent.h>
#include <libinput-cpp/UdevDevice.h>
#include <linux/uinput.h>
#include <sys/inotify.h>

namespace InputActions
{

static constexpr uint32_t MAX_INITIALIZATION_ATTEMPTS = 5;

StandaloneInputBackend::StandaloneInputBackend()
{
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
    const auto *emitter = std::dynamic_pointer_cast<EvdevInputEmitter>(g_inputEmitter).get();
    for (const auto &[device, data] : m_devices) {
        if (path == emitter->keyboardPath() || path == emitter->mousePath()
            || (data->libinputEventInjectionDevice && path == data->libinputEventInjectionDevice->devNode())
            || (data->outputDevice && path == data->outputDevice->devNode())) {
            return true;
        }
    }

    auto data = std::make_unique<ExtraDeviceData>();
    data->path = path;
    data->libinputDevice = data->libinput.addDevice(path);
    if (!data->libinputDevice) {
        // May fail if opened before the udev rule sets ACLs, initialization will be attempted again later.
        return errno == ENOENT;
    }

    const auto name = data->libinputDevice->name();
    const auto sysName = data->libinputDevice->sysName();
    InputDeviceType deviceType{};
    const auto udevDevice = data->libinputDevice->udevDevice();
    if (udevDevice->propertyValue("ID_INPUT_MOUSE")) {
        deviceType = InputDeviceType::Mouse;
    } else if (udevDevice->propertyValue("ID_INPUT_KEYBOARD")) {
        deviceType = InputDeviceType::Keyboard;
    } else if (udevDevice->propertyValue("ID_INPUT_TOUCHPAD")) {
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
        data->libevdev = LibevdevDevice::createFromPath(path);
        if (!data->libevdev) {
            return false;
        }

        if (!isDeviceNeutral(device.get(), data.get())) {
            qWarning(INPUTACTIONS).noquote().nospace()
                << QString("Failed to initialize device '%1': device is not in a neutral state and cannot be grabbed").arg(device->name());
            return false;
        }
        data->libevdev->grab();
        connect(data->libevdev.get(), &LibevdevDevice::eventsAvailable, this, &StandaloneInputBackend::poll);

        data->libinputEventInjectionDevice = LibevdevUinputDevice::createManaged(data->libevdev.get(), name + " (InputActions internal)");
        if (!data->libinputEventInjectionDevice) {
            return false;
        }

        data->libinputEventInjectionDevice->removeNonBlockFlag();
        data->libinput.removeDevice(data->libinputDevice);
        data->libinput.setGrab(true);
        data->libinputDevice = data->libinput.addDevice(data->libinputEventInjectionDevice->devNode());
        // If libinputDevice is nullptr, initialization will be reattempted later.

        data->outputDevice = LibevdevUinputDevice::createManaged(data->libevdev.get(), name + " (InputActions output)");
        if (!data->outputDevice) {
            return false;
        }

        if (deviceType == InputDeviceType::Touchpad) {
            LibevdevComplementaryInputBackend::addDevice(device.get(), data->libevdev, false);

            connect(&data->touchpadStateResetTimer, &QTimer::timeout, this, [this, device = device.get(), data = data.get()]() {
                resetDevice(device, data);
            });
        }
    } else {
        LibevdevComplementaryInputBackend::deviceAdded(device.get());
    }

    if (data->libinputDevice) {
        finishLibinputDeviceInitialization(device.get(), data.get());
    }

    connect(&data->libinput, &LibinputPathContext::eventsAvailable, this, &StandaloneInputBackend::poll);

    InputBackend::deviceAdded(device.get());
    m_devices.emplace_back(std::move(device), std::move(data));
    return true;
}

void StandaloneInputBackend::finishLibinputDeviceInitialization(InputDevice *device, ExtraDeviceData *data)
{
    if (device->type() == InputDeviceType::Touchpad) {
        data->libinputDevice->configTapSetEnabled(true);
    }

    data->libinput.dispatch();
    // Drain events
    while (const auto libinputEvent = data->libinput.getEvent()) {
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
            data->libinputDevice = data->libinput.addDevice(data->libinputEventInjectionDevice->devNode());
            if (data->libinputDevice) {
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

bool StandaloneInputBackend::handleEvent(InputDevice *sender, const LibinputEvent *event)
{
    const auto type = event->type();
    switch (type) {
        case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
        case LIBINPUT_EVENT_GESTURE_HOLD_END:
        case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
        case LIBINPUT_EVENT_GESTURE_PINCH_END:
        case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
        case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
        case LIBINPUT_EVENT_GESTURE_SWIPE_END:
        case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE: {
            const auto gestureEvent = event->gestureEvent();
            const auto fingers = gestureEvent->fingerCount();

            auto cancelled = false;
            switch (type) {
                case LIBINPUT_EVENT_GESTURE_HOLD_END:
                case LIBINPUT_EVENT_GESTURE_PINCH_END:
                case LIBINPUT_EVENT_GESTURE_SWIPE_END:
                    cancelled = gestureEvent->cancelled();
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
                    return touchpadPinchUpdate(sender, gestureEvent->scale(), gestureEvent->angleDelta());
                }
                case LIBINPUT_EVENT_GESTURE_PINCH_END:
                    return touchpadPinchEnd(sender, cancelled);
                case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
                    return touchpadSwipeBegin(sender, fingers);
                case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE: {
                    return touchpadSwipeUpdate(sender, {gestureEvent->delta(), gestureEvent->deltaUnaccelerated()});
                }
                case LIBINPUT_EVENT_GESTURE_SWIPE_END:
                    return touchpadSwipeEnd(sender, cancelled);
            }
            break;
        }
        case LIBINPUT_EVENT_KEYBOARD_KEY: {
            const auto keyboardEvent = event->keyboardEvent();

            sender->setKeyState(keyboardEvent->key(), keyboardEvent->state());
            return keyboardKey(sender, keyboardEvent->key(), keyboardEvent->state());
        }
        case LIBINPUT_EVENT_POINTER_AXIS:
        case LIBINPUT_EVENT_POINTER_BUTTON:
        case LIBINPUT_EVENT_POINTER_MOTION:
            const auto pointerEvent = event->pointerEvent();
            switch (type) {
                case LIBINPUT_EVENT_POINTER_AXIS: {
                    static const auto axis = [](const auto &pointerEvent, const auto axis) {
                        return pointerEvent->hasAxis(axis) ? pointerEvent->axisValue(axis) : 0;
                    };
                    const QPointF delta(axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL), axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
                    return pointerAxis(sender, delta);
                }
                case LIBINPUT_EVENT_POINTER_BUTTON: {
                    return pointerButton(sender, scanCodeToMouseButton(pointerEvent->button()), pointerEvent->button(), pointerEvent->state());
                }
                case LIBINPUT_EVENT_POINTER_MOTION:
                    return pointerMotion(sender, {pointerEvent->delta(), pointerEvent->deltaUnaccelerated()});
            }
    }
    return false;
}

LibinputEventsProcessingResult StandaloneInputBackend::handleLibinputEvents(InputDevice *device, LibinputPathContext &libinput)
{
    libinput.dispatch();

    // FIXME: One evdev frame can result in multiple libinput events, but one blocked libinput event will block the entire evdev frame.
    LibinputEventsProcessingResult result;
    while (const auto event = libinput.getEvent()) {
        result.block = handleEvent(device, event.get()) || result.block;
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
                if (data->libevdev->hasEventCode(EV_KEY, code) && data->libevdev->eventValue(EV_KEY, code)) {
                    return false;
                }
            }
            break;
        case InputDeviceType::Touchpad:
            return !data->libevdev->hasEventCode(EV_KEY, BTN_TOUCH) || !data->libevdev->eventValue(EV_KEY, BTN_TOUCH);
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
                if (data->libevdev->hasEventCode(EV_KEY, code) && data->libevdev->eventValue(EV_KEY, code)) {
                    syn = true;
                    data->outputDevice->writeEvent(EV_KEY, code, 0);
                }
            }

            if (syn) {
                data->outputDevice->writeSynReportEvent();
            }
            break;
        }
        case InputDeviceType::Touchpad:
            // Reverse order so that ABS_MT_SLOT is equal to 0 after
            for (int i = device->touchPoints().size() - 1; i >= 0; i--) {
                data->outputDevice->writeEvent(EV_ABS, ABS_MT_SLOT, i);
                data->outputDevice->writeEvent(EV_ABS, ABS_MT_TRACKING_ID, -1);
            }

            data->outputDevice->writeEvent(EV_KEY, BTN_TOOL_QUINTTAP, 0);
            data->outputDevice->writeEvent(EV_KEY, BTN_TOOL_QUADTAP, 0);
            data->outputDevice->writeEvent(EV_KEY, BTN_TOOL_TRIPLETAP, 0);
            data->outputDevice->writeEvent(EV_KEY, BTN_TOUCH, 0);
            data->outputDevice->writeEvent(EV_KEY, BTN_TOOL_DOUBLETAP, 0);
            data->outputDevice->writeEvent(EV_KEY, BTN_TOOL_FINGER, 0);
            data->outputDevice->writeEvent(EV_ABS, ABS_PRESSURE, 0);
            data->outputDevice->writeSynReportEvent();
            break;
    }
}

void StandaloneInputBackend::copyTouchpadState(const ExtraDeviceData *data) const
{
    for (int code = 0; code < KEY_MAX; code++) {
        if (!data->libevdev->hasEventCode(EV_KEY, code)) {
            continue;
        }

        data->outputDevice->writeEvent(EV_KEY, code, data->libevdev->eventValue(EV_KEY, code));
    }

    for (int code = 0; code < ABS_MAX; code++) {
        if ((code >= ABS_MT_SLOT && code <= ABS_MT_TOOL_Y) || !data->libevdev->hasEventCode(EV_ABS, code)) {
            continue;
        }

        data->outputDevice->writeEvent(EV_ABS, code, data->libevdev->absInfo(code)->value);
    }

    for (int slot = 0; slot < data->libevdev->slotCount(); slot++) {
        data->outputDevice->writeEvent(EV_ABS, ABS_MT_SLOT, slot);

        for (int code = ABS_MT_SLOT; code <= ABS_MT_TOOL_Y; code++) {
            if (code == ABS_MT_SLOT || !data->libevdev->hasEventCode(EV_ABS, code)) {
                continue;
            }

            data->outputDevice->writeEvent(EV_ABS, code, data->libevdev->slotValue(slot, code));
        }
    }

    data->outputDevice->writeEvent(EV_ABS, ABS_MT_SLOT, data->libevdev->currentSlot());
    data->outputDevice->writeSynReportEvent();
}

void StandaloneInputBackend::poll()
{
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
            status = data->libevdev->nextEvent(flags, evdevEvent);
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
                data->libinputEventInjectionDevice->writeEvent(event.type, event.code, event.value);
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
                    data->outputDevice->writeEvent(event.type, event.code, event.value);
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

LibevdevUinputDevice *StandaloneInputBackend::outputDevice(const InputDevice *device) const
{
    for (const auto &[libinputactionsDevice, data] : m_devices) {
        if (libinputactionsDevice.get() == device) {
            return data->outputDevice.get();
        }
    }
    return nullptr;
}

StandaloneInputBackend::ExtraDeviceData::ExtraDeviceData() = default;
StandaloneInputBackend::ExtraDeviceData::~ExtraDeviceData() = default;

}