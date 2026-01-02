/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

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

#include "KWinInputDevice.h"

namespace InputActions
{

KWinInputDevice::KWinInputDevice(KWin::InputDevice *device, InputDeviceType type)
    : InputDevice(type, device->name(), device->property("sysName").toString())
    , m_kwinDevice(device)
{
    if (device->property("lmrTapButtonMap").value<bool>()) {
        properties().setLmrTapButtonMap(true);
    }
}

std::unique_ptr<KWinInputDevice> KWinInputDevice::tryCreate(KWin::InputDevice *device)
{
    InputDeviceType type;
    if (device->isPointer() && !device->isTouch() && !device->isTouchpad()) {
        type = InputDeviceType::Mouse;
    } else if (device->isKeyboard()) {
        type = InputDeviceType::Keyboard;
    } else if (device->isTouchpad()) {
        type = InputDeviceType::Touchpad;
    } else {
        return {};
    }

    return std::unique_ptr<KWinInputDevice>(new KWinInputDevice(device, type));
}

}