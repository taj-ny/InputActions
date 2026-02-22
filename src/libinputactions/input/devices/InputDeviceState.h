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

#include <QPointF>
#include <chrono>
#include <libinputactions/input/KeyboardKey.h>
#include <set>

namespace InputActions
{

enum TouchPointType
{
    None,
    Finger,
    Thumb,
    Palm
};

struct TouchPoint
{
    /**
     * Whether this touch point is active and fits within the pressure ranges.
     */
    bool valid{};
    TouchPointType type = TouchPointType::None;
    /**
     * May be unused.
     */
    int32_t id{};

    /**
     * Whether this touch point is active.
     */
    bool active{};

    // These members must not be reset if the point becomes invalid or inactive.
    /**
     * Raw position provided by the compositor or evdev. Required for simulating taps. Only used for touchscreens.
     */
    QPointF rawPosition;
    /**
     * Raw position provided by the compositor or evdev. Required for simulating taps. Only used for touchscreens.
     */
    QPointF rawInitialPosition;

    QPointF position;
    QPointF initialPosition;
    uint32_t pressure{};
    std::chrono::steady_clock::time_point downTimestamp;
};

class InputDeviceState
{
public:
    /**
     * Derived from pressed keyboard keys.
     */
    Qt::KeyboardModifiers activeKeyboardModifiers() const;
    const std::set<KeyboardKey> &pressedKeys() const { return m_keys; }
    bool isKeyPressed(KeyboardKey key) const;

    void setKeyState(KeyboardKey key, bool state);

    const std::vector<TouchPoint> &touchPoints() const { return m_touchPoints; }
    std::vector<TouchPoint> &touchPoints() { return m_touchPoints; }
    const TouchPoint *findTouchPoint(int32_t id) const;
    TouchPoint *findTouchPoint(int32_t id);

    std::vector<const TouchPoint *> validTouchPoints() const;

private:
    std::set<KeyboardKey> m_keys;
    std::vector<TouchPoint> m_touchPoints;
};

}