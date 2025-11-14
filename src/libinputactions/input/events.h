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

#include "Delta.h"
#include "InputDevice.h"
#include <QKeyCombination>
#include <QPointF>
#include <libinputactions/globals.h>

namespace InputActions
{

enum class InputEventType
{
    KeyboardKey,

    PointerAxis,
    PointerButton,
    PointerMotion,

    TouchDown,
    TouchChanged,
    TouchUp,

    TouchpadClick,
    TouchpadGestureLifecyclePhase,
    TouchpadSlot,
    TouchpadSwipe,
    TouchpadPinch
};

enum class TouchpadGestureLifecyclePhase
{
    Begin,
    Cancel,
    End
};

class InputEvent
{
public:
    virtual ~InputEvent() = default;

    const InputEventType &type() const;
    InputDevice *sender() const;

protected:
    InputEvent(InputEventType type, InputDevice *sender);

private:
    InputEventType m_type;
    InputDevice *m_sender;
};

class MotionEvent : public InputEvent
{
public:
    MotionEvent(InputDevice *sender, InputEventType type, PointDelta delta);

    const PointDelta &delta() const;

private:
    PointDelta m_delta;
};

class KeyboardKeyEvent : public InputEvent
{
public:
    KeyboardKeyEvent(InputDevice *sender, uint32_t nativeKey, bool state);

    const uint32_t &nativeKey() const;
    const bool &state() const;

private:
    uint32_t m_nativeKey;
    bool m_state;
};

class PointerButtonEvent : public InputEvent
{
public:
    PointerButtonEvent(InputDevice *sender, Qt::MouseButton button, uint32_t nativeButton, bool state);

    const Qt::MouseButton &button() const;
    const uint32_t &nativeButton() const;
    const bool &state() const;

private:
    Qt::MouseButton m_button;
    uint32_t m_nativeButton;
    bool m_state;
};

class TouchpadClickEvent : public InputEvent
{
public:
    TouchpadClickEvent(InputDevice *sender, bool state);

    const bool &state() const;

private:
    bool m_state;
};

class TouchpadPinchEvent : public InputEvent
{
public:
    TouchpadPinchEvent(InputDevice *sender, qreal scale, qreal angleDelta);

    const qreal &scale() const;
    const qreal &angleDelta() const;

private:
    qreal m_scale;
    qreal m_angleDelta;
};

class TouchpadGestureLifecyclePhaseEvent : public InputEvent
{
public:
    TouchpadGestureLifecyclePhaseEvent(InputDevice *sender, TouchpadGestureLifecyclePhase phase, TriggerTypes triggers, uint8_t fingers = 0);

    const TouchpadGestureLifecyclePhase &phase() const;
    const TriggerTypes &triggers() const;
    const uint8_t &fingers() const;

private:
    TouchpadGestureLifecyclePhase m_phase;
    TriggerTypes m_triggers;
    uint8_t m_fingers;
};

class TouchEvent : public InputEvent
{
public:
    TouchEvent(InputDevice *sender, InputEventType type, TouchPoint point);

    const TouchPoint &point() const;

private:
    TouchPoint m_point;
};

class TouchChangedEvent : public TouchEvent
{
public:
    TouchChangedEvent(InputDevice *sender, TouchPoint point, QPointF positionDelta);

    const QPointF &positionDelta() const;

private:
    QPointF m_positionDelta;
};

}