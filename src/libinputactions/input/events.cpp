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

#include "events.h"

namespace InputActions
{

InputEvent::InputEvent(InputEventType type, InputDevice *sender)
    : m_type(type)
    , m_sender(sender)
{
}

EvdevEvent::EvdevEvent(uint16_t type, uint16_t code, int32_t value)
    : m_type(type)
    , m_code(code)
    , m_value(value)
{
}

EvdevFrameEvent::EvdevFrameEvent(InputDevice *sender, std::vector<EvdevEvent> events)
    : InputEvent(InputEventType::EvdevFrame, sender)
    , m_events(std::move(events))
{
}

MotionEvent::MotionEvent(InputDevice *sender, InputEventType type, PointDelta delta, bool oneAxisPerEvent)
    : InputEvent(type, sender)
    , m_delta(std::move(delta))
    , m_oneAxisPerEvent(oneAxisPerEvent)
{
}

PointerButtonEvent::PointerButtonEvent(InputDevice *sender, Qt::MouseButton button, uint32_t nativeButton, bool state)
    : InputEvent(InputEventType::PointerButton, sender)
    , m_button(button)
    , m_nativeButton(nativeButton)
    , m_state(state)
{
}

KeyboardKeyEvent::KeyboardKeyEvent(InputDevice *sender, uint32_t nativeKey, bool state)
    : InputEvent(InputEventType::KeyboardKey, sender)
    , m_nativeKey(nativeKey)
    , m_state(state)
{
}

TouchpadClickEvent::TouchpadClickEvent(InputDevice *sender, bool state)
    : InputEvent(InputEventType::TouchpadClick, sender)
    , m_state(state)
{
}

TouchpadPinchEvent::TouchpadPinchEvent(InputDevice *sender, qreal scale, qreal angleDelta)
    : InputEvent(InputEventType::TouchpadPinch, sender)
    , m_scale(scale)
    , m_angleDelta(angleDelta)
{
}

TouchpadGestureLifecyclePhaseEvent::TouchpadGestureLifecyclePhaseEvent(InputDevice *sender, TouchpadGestureLifecyclePhase phase, TriggerTypes triggers,
                                                                       uint8_t fingers)
    : InputEvent(InputEventType::TouchpadGestureLifecyclePhase, sender)
    , m_phase(phase)
    , m_triggers(triggers)
    , m_fingers(fingers)
{
}

TouchDownEvent::TouchDownEvent(InputDevice *sender, int32_t id, QPointF position, QPointF rawPosition, uint32_t pressure)
    : InputEvent(InputEventType::TouchDown, sender)
    , m_id(id)
    , m_position(position)
    , m_rawPosition(rawPosition)
    , m_pressure(pressure)
{
}

TouchMotionEvent::TouchMotionEvent(InputDevice *sender, int32_t id, QPointF position, QPointF rawPosition)
    : InputEvent(InputEventType::TouchMotion, sender)
    , m_id(id)
    , m_position(position)
    , m_rawPosition(rawPosition)
{
}

TouchPressureChangeEvent::TouchPressureChangeEvent(InputDevice *sender, int32_t id, uint32_t pressure)
    : InputEvent(InputEventType::TouchPressureChange, sender)
    , m_id(id)
    , m_pressure(pressure)
{
}

TouchUpEvent::TouchUpEvent(InputDevice *sender, int32_t id)
    : InputEvent(InputEventType::TouchUp, sender)
    , m_id(id)
{
}

TouchCancelEvent::TouchCancelEvent(InputDevice *sender)
    : InputEvent(InputEventType::TouchCancel, sender)
{
}

TouchFrameEvent::TouchFrameEvent(InputDevice *sender)
    : InputEvent(InputEventType::TouchFrame, sender)
{
}

}