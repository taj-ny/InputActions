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

#include <QKeyCombination>
#include <QPointF>

namespace libinputactions
{

enum class InputEventType
{
    KeyboardKey,

    MouseButton,
    MouseMotion,
    MouseWheel,

    TouchpadGestureLifecyclePhase,
    TouchpadScroll,
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
    const std::optional<QString> &sender() const;

protected:
    InputEvent(const InputEventType &type, const std::optional<QString> &sender = {});

private:
    InputEventType m_type;
    std::optional<QString> m_sender;
};

class MotionEvent : public InputEvent
{
public:
    MotionEvent(const QPointF &delta, const InputEventType &type, const std::optional<QString> &sender = {});

    const QPointF &delta() const;

private:
    QPointF m_delta;
};

class MouseButtonEvent : public InputEvent
{
public:
    MouseButtonEvent(const Qt::MouseButton &button, const quint32 &nativeButton, const bool &state, const std::optional<QString> &sender = {});

    const Qt::MouseButton &button() const;
    const quint32 &nativeButton() const;
    const bool &state() const;

private:
    Qt::MouseButton m_button;
    quint32 m_nativeButton;
    bool m_state;
};

class KeyboardKeyEvent : public InputEvent
{
public:
    KeyboardKeyEvent(const uint32_t &nativeKey, const bool &state);

    const uint32_t &nativeKey() const;
    const bool &state() const;

private:
    uint32_t m_nativeKey;
    bool m_state;
};

class TouchpadPinchEvent : public InputEvent
{
public:
    TouchpadPinchEvent(const qreal &scale, const qreal &angleDelta);

    const qreal &scale() const;
    const qreal &angleDelta() const;

private:
    qreal m_scale;
    qreal m_angleDelta;
};

class TouchpadGestureLifecyclePhaseEvent : public InputEvent
{
public:
    TouchpadGestureLifecyclePhaseEvent(const TouchpadGestureLifecyclePhase &phase, const TriggerTypes &triggers, const uint8_t &fingers = 0);

    const TouchpadGestureLifecyclePhase &phase() const;
    const TriggerTypes &triggers() const;
    const uint8_t &fingers() const;

private:
    TouchpadGestureLifecyclePhase m_phase;
    TriggerTypes m_triggers;
    uint8_t m_fingers;
};

}