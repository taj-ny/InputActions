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

#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/devices/InputDeviceState.h>
#include <linux/input.h>
#include <map>
#include <memory>

namespace InputActions
{

class LibevdevDevice;

struct SlotData
{
    bool active{};
    QPointF position;
    uint32_t pressure{};
};

/**
 * Uses libevdev to get additional touchpad data that libinput does not provide.
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
    /**
     * Adds a device whose libevdev instance is not managed by a primary input backend. This backend will poll the device.
     */
    void addDevice(InputDevice *device);
    /**
     * Adds a device whose libevdev instance is managed by a primary input backend. This backend will not poll the device, the primary backend must call
     * handleEvdevEvent.
     */
    void addDevice(InputDevice *device, std::shared_ptr<LibevdevDevice> libevdevDevice);
    void removeDevice(const InputDevice *device) override;

    void handleEvdevEvent(InputDevice *sender, const input_event &event);

private:
    void addDevice(InputDevice *device, std::shared_ptr<LibevdevDevice> libevdevDevice, bool owner);

    bool m_enabled = true;

    struct ExtraDeviceData
    {
        ExtraDeviceData();
        ~ExtraDeviceData();

        std::shared_ptr<LibevdevDevice> device;
        /**
         * Whether the device is owned by this input backend.
         */
        bool owner{};

        /**
         * Absolute minimum values of ABS_X and ABS_Y.
         */
        QPoint absMin;

        int32_t currentSlot{};
        std::map<int32_t, SlotData> currentSlots;
        std::map<int32_t, SlotData> previousSlots;
    };
    std::map<InputDevice *, std::unique_ptr<ExtraDeviceData>> m_devices;
};
}