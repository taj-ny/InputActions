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

#include "InputTriggerHandler.h"
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/input/events.h>

namespace InputActions
{

void InputTriggerHandler::setDevice(InputDevice *device)
{
    m_device = device;
}

void InputTriggerHandler::setDeviceTypes(InputDeviceTypes types)
{
    m_types = types;
}

bool InputTriggerHandler::acceptsEvent(const InputEvent &event)
{
    return (!m_device && !m_types) || (m_device && event.sender() == m_device) || (m_types && m_types & event.sender()->type())
        || event.type() == InputEventType::KeyboardKey;
}

bool InputTriggerHandler::keyboardKey(const KeyboardKeyEvent &event)
{
    // Lazy way of detecting modifier release during mouse gestures
    if (!event.state()) {
        endTriggers(TriggerType::All);
    }
    return false;
}

}