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

const InputEventType &InputEvent::type() const
{
    return m_type;
}

InputDevice *InputEvent::sender() const
{
    return m_sender;
}

MotionEvent::MotionEvent(InputDevice *sender, InputEventType type, PointDelta delta, bool oneAxisPerEvent)
    : InputEvent(type, sender)
    , m_delta(std::move(delta))
    , m_oneAxisPerEvent(oneAxisPerEvent)
{
}

const PointDelta &MotionEvent::delta() const
{
    return m_delta;
}

bool MotionEvent::oneAxisPerEvent() const
{
    return m_oneAxisPerEvent;
}

PointerButtonEvent::PointerButtonEvent(InputDevice *sender, Qt::MouseButton button, uint32_t nativeButton, bool state)
    : InputEvent(InputEventType::PointerButton, sender)
    , m_button(button)
    , m_nativeButton(nativeButton)
    , m_state(state)
{
}

const Qt::MouseButton &PointerButtonEvent::button() const
{
    return m_button;
}

const uint32_t &PointerButtonEvent::nativeButton() const
{
    return m_nativeButton;
}

const bool &PointerButtonEvent::state() const
{
    return m_state;
}

KeyboardKeyEvent::KeyboardKeyEvent(InputDevice *sender, uint32_t nativeKey, bool state)
    : InputEvent(InputEventType::KeyboardKey, sender)
    , m_nativeKey(nativeKey)
    , m_state(state)
{
}

const uint32_t &KeyboardKeyEvent::nativeKey() const
{
    return m_nativeKey;
}

const bool &KeyboardKeyEvent::state() const
{
    return m_state;
}

TouchpadClickEvent::TouchpadClickEvent(InputDevice *sender, bool state)
    : InputEvent(InputEventType::TouchpadClick, sender)
    , m_state(state)
{
}

const bool &TouchpadClickEvent::state() const
{
    return m_state;
}

TouchpadPinchEvent::TouchpadPinchEvent(InputDevice *sender, qreal scale, qreal angleDelta)
    : InputEvent(InputEventType::TouchpadPinch, sender)
    , m_scale(scale)
    , m_angleDelta(angleDelta)
{
}

const qreal &TouchpadPinchEvent::scale() const
{
    return m_scale;
}

const qreal &TouchpadPinchEvent::angleDelta() const
{
    return m_angleDelta;
}

TouchpadGestureLifecyclePhaseEvent::TouchpadGestureLifecyclePhaseEvent(InputDevice *sender, TouchpadGestureLifecyclePhase phase, TriggerTypes triggers,
                                                                       uint8_t fingers)
    : InputEvent(InputEventType::TouchpadGestureLifecyclePhase, sender)
    , m_phase(phase)
    , m_triggers(triggers)
    , m_fingers(fingers)
{
}

const TouchpadGestureLifecyclePhase &TouchpadGestureLifecyclePhaseEvent::phase() const
{
    return m_phase;
}

const TriggerTypes &TouchpadGestureLifecyclePhaseEvent::triggers() const
{
    return m_triggers;
}

const uint8_t &TouchpadGestureLifecyclePhaseEvent::fingers() const
{
    return m_fingers;
}

TouchEvent::TouchEvent(InputDevice *sender, InputEventType type, TouchPoint point)
    : InputEvent(type, sender)
    , m_point(std::move(point))
{
}

const TouchPoint &TouchEvent::point() const
{
    return m_point;
}

TouchChangedEvent::TouchChangedEvent(InputDevice *sender, TouchPoint point, QPointF positionDelta)
    : TouchEvent(sender, InputEventType::TouchChanged, point)
    , m_positionDelta(std::move(positionDelta))
{
}

const QPointF &TouchChangedEvent::positionDelta() const
{
    return m_positionDelta;
}

}