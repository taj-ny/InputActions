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

#include "InputDeviceState.h"
#include "InputDevice.h"

namespace InputActions
{

Qt::KeyboardModifiers InputDeviceState::activeKeyboardModifiers() const
{
    Qt::KeyboardModifiers modifiers;
    for (const auto &[key, modifier] : KEYBOARD_MODIFIERS) {
        if (m_keys.contains(key)) {
            modifiers |= modifier;
        }
    }
    return modifiers;
}

bool InputDeviceState::isKeyPressed(uint32_t key) const
{
    return m_keys.contains(key);
}

void InputDeviceState::setKeyState(uint32_t key, bool state)
{
    if (state) {
        m_keys.insert(key);
    } else {
        m_keys.erase(key);
    }
}

std::vector<const TouchPoint *> InputDeviceState::validTouchPoints() const
{
    std::vector<const TouchPoint *> result;
    for (auto &point : m_touchPoints) {
        if (point.valid) {
            result.push_back(&point);
        }
    }
    std::ranges::sort(result, [](const auto *a, const auto *b) {
        return a->downTimestamp < b->downTimestamp;
    });
    return result;
}

const TouchPoint *InputDeviceState::findTouchPoint(int32_t id) const
{
    for (const auto &point : m_touchPoints) {
        if (point.id == id) {
            return &point;
        }
    }
    return {};
}

TouchPoint *InputDeviceState::findTouchPoint(int32_t id)
{
    for (auto &point : m_touchPoints) {
        if (point.id == id) {
            return &point;
        }
    }
    return {};
}

}