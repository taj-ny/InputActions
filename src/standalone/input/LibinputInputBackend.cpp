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

#include "LibinputInputBackend.h"

#include <fcntl.h>

#include <ranges>

using namespace libinputactions;

LibinputInputBackend::LibinputInputBackend()
{
    m_libinputInterface = {
        .open_restricted = &LibinputInputBackend::openRestricted,
        .close_restricted = &LibinputInputBackend::close_restricted,
    };
}

LibinputInputBackend::~LibinputInputBackend()
{
    reset();
}

void LibinputInputBackend::initialize()
{
    m_udev = udev_new();
    m_libinput = libinput_udev_create_context(&m_libinputInterface, nullptr, m_udev);
    libinput_udev_assign_seat(m_libinput, "seat0");
}

void LibinputInputBackend::reset()
{
    for (auto &device : m_devices) {
        deviceRemoved(device.libinputactionsDevice.get());
        libinput_device_unref(device.libinputDevice);
    }
    m_devices.clear();
    if (m_libinput) {
        libinput_unref(m_libinput);
    }
    if (m_udev) {
        udev_unref(m_udev);
    }
    LibinputIndirectInputBackend::reset();
}

void LibinputInputBackend::poll()
{
    LibevdevComplementaryInputBackend::poll();
    libinput_dispatch(m_libinput);
    libinput_event *event;
    while ((event = libinput_get_event(m_libinput))) {
        const auto type = libinput_event_get_type(event);
        auto *libinputDevice = libinput_event_get_device(event);
        auto device = std::ranges::find_if(m_devices, [libinputDevice](auto &device) {
            return device.libinputDevice == libinputDevice;
        });

        if (type != LIBINPUT_EVENT_DEVICE_ADDED && device == m_devices.end()) {
            continue;
        }

        switch (type) {
            case LIBINPUT_EVENT_DEVICE_ADDED: {
                libinput_device_ref(libinputDevice);

                const QString name(libinput_device_get_name(libinputDevice));
                const QString sysName(libinput_device_get_sysname(libinputDevice));
                InputDeviceType deviceType;
                auto udevDevice = libinput_device_get_udev_device(libinputDevice);
                if (udev_device_get_property_value(udevDevice, "ID_INPUT_TOUCHPAD")) {
                    deviceType = InputDeviceType::Touchpad;
                }

                auto libinputactionsDevice = std::make_unique<InputDevice>(deviceType, name, sysName);
                deviceAdded(libinputactionsDevice.get());
                m_devices.push_back({
                    .libinputDevice = libinputDevice,
                    .libinputactionsDevice = std::move(libinputactionsDevice)
                });
                break;
            }
            case LIBINPUT_EVENT_DEVICE_REMOVED:
                libinput_device_unref(libinputDevice);
                deviceRemoved(device->libinputactionsDevice.get());
                m_devices.erase(device);
                break;
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
                }

                switch (type) {
                    case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
                        touchpadHoldBegin(device->libinputactionsDevice.get(), fingers);
                        break;
                    case LIBINPUT_EVENT_GESTURE_HOLD_END:
                        touchpadHoldEnd(device->libinputactionsDevice.get(), cancelled);
                        break;
                    case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
                        touchpadPinchBegin(device->libinputactionsDevice.get(), fingers);
                        break;
                    case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE: {
                        const auto scale = libinput_event_gesture_get_scale(gestureEvent);
                        const auto angleDelta = libinput_event_gesture_get_angle_delta(gestureEvent);
                        touchpadPinchUpdate(device->libinputactionsDevice.get(), scale, angleDelta);
                        break;
                    }
                    case LIBINPUT_EVENT_GESTURE_PINCH_END:
                        touchpadPinchEnd(device->libinputactionsDevice.get(), cancelled);
                        break;
                    case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
                        touchpadSwipeBegin(device->libinputactionsDevice.get(), fingers);
                        break;
                    case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE: {
                        const QPointF delta(libinput_event_gesture_get_dx_unaccelerated(gestureEvent),
                            libinput_event_gesture_get_dy_unaccelerated(gestureEvent));
                        touchpadSwipeUpdate(device->libinputactionsDevice.get(), delta);
                        break;
                    }
                    case LIBINPUT_EVENT_GESTURE_SWIPE_END:
                        touchpadSwipeEnd(device->libinputactionsDevice.get(), cancelled);
                        break;
                }
                break;
            }
            case LIBINPUT_EVENT_POINTER_AXIS:
            case LIBINPUT_EVENT_POINTER_BUTTON:
                auto *pointerEvent = libinput_event_get_pointer_event(event);
                switch (type) {
                    case LIBINPUT_EVENT_POINTER_AXIS: {
                        static const auto axis = [](auto *pointerEvent, auto axis) {
                            return libinput_event_pointer_has_axis(pointerEvent, axis)
                                ? libinput_event_pointer_get_axis_value(pointerEvent, axis)
                                : 0;
                        };
                        const QPointF delta(axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL),
                            axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
                        pointerAxis(device->libinputactionsDevice.get(), delta);
                        break;
                    }
                    case LIBINPUT_EVENT_POINTER_BUTTON:
                        const auto button = libinput_event_pointer_get_button(pointerEvent);
                        const auto state = libinput_event_pointer_get_button_state(pointerEvent);
                        pointerButton(device->libinputactionsDevice.get(), scanCodeToMouseButton(button), button, state == LIBINPUT_BUTTON_STATE_PRESSED);
                        break;
                }
                break;
        }
        libinput_event_destroy(event);
    }
}

int LibinputInputBackend::openRestricted(const char *path, int flags, void *data)
{
    int fd = open(path, flags);
    return fd < 0 ? -errno : fd;
}

void LibinputInputBackend::close_restricted(int fd, void *data)
{
    close(fd);
}