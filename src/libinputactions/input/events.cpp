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

namespace libinputactions
{

InputEvent::InputEvent(const InputEventType &type, const std::optional<QString> &sender)
    : m_type(type)
    , m_sender(sender)
{
}

const InputEventType &InputEvent::type() const
{
    return m_type;
}

const std::optional<QString> &InputEvent::sender() const
{
    return m_sender;
}

MotionEvent::MotionEvent(const QPointF &delta, const InputEventType &type, const std::optional<QString> &sender)
    : InputEvent(type, sender)
    , m_delta(delta)
{
}

const QPointF &MotionEvent::delta() const
{
    return m_delta;
}

MouseButtonEvent::MouseButtonEvent(const Qt::MouseButton &button, const quint32 &nativeButton, const bool &state,
    const std::optional<QString> &sender)
    : InputEvent(InputEventType::MouseButton, sender)
    , m_button(button)
    , m_nativeButton(nativeButton)
    , m_state(state)
{
}

const Qt::MouseButton &MouseButtonEvent::button() const
{
    return m_button;
}

const quint32 &MouseButtonEvent::nativeButton() const
{
    return m_nativeButton;
}

const bool &MouseButtonEvent::state() const
{
    return m_state;
}

KeyboardKeyEvent::KeyboardKeyEvent(const uint32_t &nativeKey, const bool &state)
    : InputEvent(InputEventType::KeyboardKey)
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

TouchpadPinchEvent::TouchpadPinchEvent(const qreal &scale, const qreal &angleDelta)
    : InputEvent(InputEventType::TouchpadPinch)
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

TouchpadGestureLifecyclePhaseEvent::TouchpadGestureLifecyclePhaseEvent(const TouchpadGestureLifecyclePhase &phase,
    const TriggerTypes &triggers,
    const uint8_t &fingers)
    : InputEvent(InputEventType::TouchpadGestureLifecyclePhase)
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

TouchpadSlotEvent::TouchpadSlotEvent(const std::vector<TouchpadSlot> &fingerSlots)
    : InputEvent(InputEventType::TouchpadSlot)
    , m_slots(fingerSlots)
{
}

const std::vector<TouchpadSlot> &TouchpadSlotEvent::fingerSlots() const
{
    return m_slots;
}

}