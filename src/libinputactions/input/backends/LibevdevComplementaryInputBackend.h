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

#pragma once

#include <QSocketNotifier>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/events.h>
#include <map>
#include <memory>

namespace InputActions
{

/**
 * Uses libevdev to get additional touchpad data that libinput does not provide.
 *
 * Emitted events: TouchpadClick, TouchpadSlot
 */
class LibevdevComplementaryInputBackend : public InputBackend
{
public:
    LibevdevComplementaryInputBackend() = default;

    void poll(InputDevice *device);

    /**
     * Will take effect only if set before initialization.
     */
    void setEnabled(bool value);

protected:
    void deviceAdded(InputDevice *device) override;
    void addDevice(InputDevice *device, libevdev *libevdev, bool owner);

    void deviceRemoved(const InputDevice *device) override;

    void handleEvdevEvent(InputDevice *sender, const input_event &event);

private:
    libevdev *openDevice(const QString &sysName);

    bool m_enabled = true;

    struct ExtraDeviceData
    {
        ~ExtraDeviceData();

        struct libevdev *libevdev{};
        /**
         * Whether libevdev is owned by this input backend.
         */
        bool owner{};
        std::unique_ptr<QSocketNotifier> notifier;

        uint8_t currentSlot{};
        /**
         * Absolute minimum values of ABS_X and ABS_Y.
         */
        QPoint absMin;
        /**
         * Copy of touch points from the previous frame.
         */
        std::vector<TouchPoint> previousTouchPoints;
    };
    std::map<InputDevice *, std::unique_ptr<ExtraDeviceData>> m_devices;
};
}