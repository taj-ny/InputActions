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

#include "device.h"

#include <libinputactions/globals.h>

#include <QKeyCombination>
#include <QPointF>

namespace libinputactions
{

enum class InputEventType
{
    KeyboardKey,

    PointerButton,
    PointerMotion,
    PointerScroll,

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
    const InputDevice &sender() const;

protected:
    InputEvent(const InputEventType &type, const InputDevice &sender);

private:
    InputEventType m_type;
    InputDevice m_sender;
};

class MotionEvent : public InputEvent
{
public:
    MotionEvent(const InputDevice &sender, const InputEventType &type, const QPointF &delta);

    const QPointF &delta() const;

private:
    QPointF m_delta;
};

class KeyboardKeyEvent : public InputEvent
{
public:
    KeyboardKeyEvent(const InputDevice &sender, const uint32_t &nativeKey, const bool &state);

    const uint32_t &nativeKey() const;
    const bool &state() const;

private:
    uint32_t m_nativeKey;
    bool m_state;
};

class PointerButtonEvent : public InputEvent
{
public:
    PointerButtonEvent(const InputDevice &sender, const Qt::MouseButton &button, const quint32 &nativeButton, const bool &state);

    const Qt::MouseButton &button() const;
    const quint32 &nativeButton() const;
    const bool &state() const;

private:
    Qt::MouseButton m_button;
    quint32 m_nativeButton;
    bool m_state;
};

class TouchpadClickEvent : public InputEvent
{
public:
    TouchpadClickEvent(const InputDevice &sender, const bool &state);

    const bool &state() const;

private:
    bool m_state;
};

class TouchpadPinchEvent : public InputEvent
{
public:
    TouchpadPinchEvent(const InputDevice &sender, const qreal &scale, const qreal &angleDelta);

    const qreal &scale() const;
    const qreal &angleDelta() const;

private:
    qreal m_scale;
    qreal m_angleDelta;
};

class TouchpadGestureLifecyclePhaseEvent : public InputEvent
{
public:
    TouchpadGestureLifecyclePhaseEvent(const InputDevice &sender, const TouchpadGestureLifecyclePhase &phase, const TriggerTypes &triggers, const uint8_t &fingers = 0);

    const TouchpadGestureLifecyclePhase &phase() const;
    const TriggerTypes &triggers() const;
    const uint8_t &fingers() const;

private:
    TouchpadGestureLifecyclePhase m_phase;
    TriggerTypes m_triggers;
    uint8_t m_fingers;
};

struct TouchpadSlot
{
    bool active{};
    QPointF position;
    uint32_t pressure{};
};

class TouchpadSlotEvent : public InputEvent
{
public:
    TouchpadSlotEvent(const InputDevice &sender, const std::vector<TouchpadSlot> &fingerSlots);

    const std::vector<TouchpadSlot> &fingerSlots() const;
private:
    std::vector<TouchpadSlot> m_slots;
};

}