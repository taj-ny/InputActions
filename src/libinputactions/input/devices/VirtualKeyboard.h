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

#pragma once

#include <QString>
#include <libinputactions/input/KeyboardKey.h>
#include <set>

namespace InputActions
{

/**
 * Virtual device for emitting anonymous keyboard events.
 *
 * All keys that will be used must be registered using InputBackend::addVirtualKeyboardKey() prior to initializing the input backend. The implementation can
 * then obtain them using InputBackend::virtualKeyboardKeys().
 */
class VirtualKeyboard
{
public:
    virtual ~VirtualKeyboard() = default;

    /**
     * Must be called by the overriding method in order to track pressed keys.
     */
    virtual void keyboardKey(KeyboardKey key, bool state);
    virtual void keyboardText(const QString &text) {}

protected:
    /**
     * Puts the device in a neutral state. Call in the deriving class' destructor.
     */
    void reset();

private:
    std::set<KeyboardKey> m_pressedKeys;
};

}