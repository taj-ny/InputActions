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

#include <libinputactions/globals.h>
#include <memory>
#include <set>

namespace InputActions
{

class InputDevice;

class InputEmitter
{
public:
    InputEmitter();
    virtual ~InputEmitter() = default;

    virtual void initialize() {}
    virtual void reset() {}

    /**
     * The implementation may require that all keys that will be used must be registered before initialization. Modifier keys are added by default.
     */
    void keyboardAddRequiredKey(uint32_t key);

    virtual void keyboardClearModifiers() {}
    /**
     * @param key See <linux/input-event-codes.h>. If the key is not in m_keyboardRequiredKeys, the call may fail.
     * @param state True - press, false - release
     */
    virtual void keyboardKey(uint32_t key, bool state, InputDevice *target = nullptr) {}
    virtual void keyboardText(const QString &text) {}

    /**
     * @param delta Both X and Y values may be specified.
     */
    virtual void mouseAxis(const QPointF &delta) {}
    /**
     * @param button <linux/input-event-codes.h>
     * @param state True - press, false - release
     */
    virtual void mouseButton(uint32_t button, bool state, InputDevice *target = nullptr) {}
    virtual void mouseMoveRelative(const QPointF &pos) {}

protected:
    std::set<uint32_t> m_keyboardRequiredKeys;
};

inline std::shared_ptr<InputEmitter> g_inputEmitter;

}