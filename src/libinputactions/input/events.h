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
#include <QKeyCombination>
#include <QPointF>
#include <libinputactions/globals.h>

namespace InputActions
{

class InputDevice;

enum class InputEventType
{
    EvdevFrame,

    KeyboardKey,

    PointerAxis,
    PointerButton,
    PointerMotion,

    TouchCancel,
    TouchDown,
    TouchFrame,
    TouchMotion,
    TouchPressureChange,
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

    const InputEventType &type() const { return m_type; }
    InputDevice *sender() const { return m_sender; }

protected:
    InputEvent(InputEventType type, InputDevice *sender);

private:
    InputEventType m_type;
    InputDevice *m_sender;
};

class EvdevEvent
{
public:
    EvdevEvent(uint16_t type, uint16_t code, int32_t value);

    uint16_t type() const { return m_type; }
    uint16_t code() const { return m_code; }
    int32_t value() const { return m_value; }

private:
    uint16_t m_type;
    uint16_t m_code;
    int32_t m_value;
};

class EvdevFrameEvent : public InputEvent
{
public:
    EvdevFrameEvent(InputDevice *sender, std::vector<EvdevEvent> events);

    /**
     * Always ends with {EV_SYN, SYN_REPORT, 0}.
     */
    const std::vector<EvdevEvent> &events() const { return m_events; }

private:
    std::vector<EvdevEvent> m_events;
};

class MotionEvent : public InputEvent
{
public:
    MotionEvent(InputDevice *sender, InputEventType type, PointDelta delta, bool oneAxisPerEvent = false);

    const PointDelta &delta() const { return m_delta; }
    bool oneAxisPerEvent() const { return m_oneAxisPerEvent; }

private:
    PointDelta m_delta;
    bool m_oneAxisPerEvent;
};

class KeyboardKeyEvent : public InputEvent
{
public:
    KeyboardKeyEvent(InputDevice *sender, uint32_t nativeKey, bool state);

    uint32_t nativeKey() const { return m_nativeKey; }
    bool state() const { return m_state; }

private:
    uint32_t m_nativeKey;
    bool m_state;
};

class PointerButtonEvent : public InputEvent
{
public:
    PointerButtonEvent(InputDevice *sender, Qt::MouseButton button, uint32_t nativeButton, bool state);

    const Qt::MouseButton &button() const { return m_button; }
    uint32_t nativeButton() const { return m_nativeButton; }
    bool state() const { return m_state; }

private:
    Qt::MouseButton m_button;
    uint32_t m_nativeButton;
    bool m_state;
};

class TouchpadClickEvent : public InputEvent
{
public:
    TouchpadClickEvent(InputDevice *sender, bool state);

    bool state() const { return m_state; }

private:
    bool m_state;
};

class TouchpadPinchEvent : public InputEvent
{
public:
    TouchpadPinchEvent(InputDevice *sender, qreal scale, qreal angleDelta);

    qreal scale() const { return m_scale; }
    qreal angleDelta() const { return m_angleDelta; }

private:
    qreal m_scale;
    qreal m_angleDelta;
};

class TouchpadGestureLifecyclePhaseEvent : public InputEvent
{
public:
    TouchpadGestureLifecyclePhaseEvent(InputDevice *sender, TouchpadGestureLifecyclePhase phase, TriggerTypes triggers, uint8_t fingers = 0);

    TouchpadGestureLifecyclePhase phase() const { return m_phase; }
    const TriggerTypes &triggers() const { return m_triggers; }
    uint8_t fingers() const { return m_fingers; }

private:
    TouchpadGestureLifecyclePhase m_phase;
    TriggerTypes m_triggers;
    uint8_t m_fingers;
};

class TouchDownEvent : public InputEvent
{
public:
    TouchDownEvent(InputDevice *sender, int32_t id, QPointF position, QPointF rawPosition, uint32_t pressure = 0);

    int32_t id() const { return m_id; }
    const QPointF &position() const { return m_position; }
    const QPointF &rawPosition() const { return m_rawPosition; }
    uint32_t pressure() const { return m_pressure; }

private:
    int32_t m_id;
    QPointF m_position;
    QPointF m_rawPosition;
    uint32_t m_pressure;
};

class TouchMotionEvent : public InputEvent
{
public:
    TouchMotionEvent(InputDevice *sender, int32_t id, QPointF position, QPointF rawPosition);

    int32_t id() const { return m_id; }
    const QPointF &position() const { return m_position; }
    const QPointF &rawPosition() const { return m_rawPosition; }

private:
    int32_t m_id;
    QPointF m_position;
    QPointF m_rawPosition;
};

class TouchPressureChangeEvent : public InputEvent
{
public:
    TouchPressureChangeEvent(InputDevice *sender, int32_t id, uint32_t pressure);

    int32_t id() const { return m_id; }
    uint32_t pressure() const { return m_pressure; }

private:
    int32_t m_id;
    uint32_t m_pressure;
};

class TouchUpEvent : public InputEvent
{
public:
    TouchUpEvent(InputDevice *sender, int32_t id);

    int32_t id() const { return m_id; }

private:
    int32_t m_id;
};

class TouchCancelEvent : public InputEvent
{
public:
    TouchCancelEvent(InputDevice *sender);
};

class TouchFrameEvent : public InputEvent
{
public:
    TouchFrameEvent(InputDevice *sender);
};

}