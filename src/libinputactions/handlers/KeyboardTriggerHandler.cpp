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

#include "KeyboardTriggerHandler.h"
#include <libinputactions/input/Keyboard.h>
#include <libinputactions/input/events.h>

namespace libinputactions
{

KeyboardTriggerHandler::KeyboardTriggerHandler()
{
    setDeviceTypes(InputDeviceType::Keyboard);
}

bool KeyboardTriggerHandler::keyboardKey(const KeyboardKeyEvent &event)
{
    InputTriggerHandler::keyboardKey(event);

    const auto isModifier = MODIFIERS.contains(event.nativeKey());
    if (event.state()) {
        m_keys.insert(event.nativeKey());
        if (m_keys.size() == 1) {
            m_firstKey = event.nativeKey();
        }
        if (!MODIFIERS.contains(m_firstKey)) {
            m_block = false;
            return false;
        }

        m_block = activateTriggers(TriggerType::KeyboardShortcut);
        return m_block && !isModifier;
    }

    m_keys.erase(event.nativeKey());
    endTriggers(TriggerType::KeyboardShortcut);
    return m_block && !isModifier;
}

std::unique_ptr<TriggerActivationEvent> KeyboardTriggerHandler::createActivationEvent() const
{
    auto event = TriggerHandler::createActivationEvent();
    event->keyboardKeys = {m_keys.begin(), m_keys.end()};
    return event;
}

}