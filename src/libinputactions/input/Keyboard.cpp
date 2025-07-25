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

#include "Keyboard.h"
#include "events.h"

namespace libinputactions
{

void Keyboard::handleEvent(const KeyboardKeyEvent &event)
{
    Qt::KeyboardModifier modifier{};
    if (MODIFIERS.contains(event.nativeKey())) {
        modifier = MODIFIERS.at(event.nativeKey());
    }
    if (!modifier) {
        return;
    }

    if (event.state()) {
        m_modifiers |= modifier;
    } else {
        m_modifiers &= ~modifier;
    }
}

const Qt::KeyboardModifiers &Keyboard::modifiers() const
{
    return m_modifiers;
}

}