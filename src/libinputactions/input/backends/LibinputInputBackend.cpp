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

#include "LibinputInputBackend.h"
#include <libinputactions/interfaces/InputEmitter.h>
#include <libinputactions/interfaces/PointerPositionGetter.h>
#include <libinputactions/interfaces/PointerPositionSetter.h>

namespace InputActions
{

static const uint32_t STROKE_RECORD_TIMEOUT = 250;

bool LibinputInputBackend::keyboardKey(InputDevice *sender, uint32_t key, bool state)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    return handleEvent(KeyboardKeyEvent(sender, key, state));
}

bool LibinputInputBackend::pointerAxis(InputDevice *sender, const QPointF &delta)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    if (sender->type() == InputDeviceType::Mouse) {
        return handleEvent(MotionEvent(sender, InputEventType::PointerAxis, {delta}));
    }

    if (m_isRecordingStroke) {
        if (delta.isNull()) {
            finishStrokeRecording();
        } else {
            m_strokePoints.push_back(delta);
        }
        return true;
    }

    if (delta.isNull() && sender->type() == InputDeviceType::Touchpad) {
        LibevdevComplementaryInputBackend::poll(); // Update clicked state, clicking cancels scrolling and generates a (0,0) event
    }
    return handleEvent(MotionEvent(sender, InputEventType::PointerAxis, {delta}));
}

bool LibinputInputBackend::pointerButton(InputDevice *sender, Qt::MouseButton button, uint32_t nativeButton, bool state)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    if (sender->type() == InputDeviceType::Touchpad) {
        LibevdevComplementaryInputBackend::poll(); // Update clicked state
    }
    return handleEvent(PointerButtonEvent(sender, button, nativeButton, state));
}

bool LibinputInputBackend::pointerMotion(InputDevice *sender, const PointDelta &delta)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    if (m_isRecordingStroke) {
        m_strokePoints.push_back(delta.accelerated()); // accelerated for backwards compatibility
        m_strokeRecordingTimeoutTimer.start(STROKE_RECORD_TIMEOUT);
        return false;
    }

    auto block = handleEvent(MotionEvent(sender, InputEventType::PointerMotion, delta));
    if (block && m_previousPointerPosition) {
        g_pointerPositionSetter->setGlobalPointerPosition(m_previousPointerPosition.value());
    }
    if (!block) {
        m_previousPointerPosition = g_pointerPositionGetter->globalPointerPosition();
    }
    return block;
}

bool LibinputInputBackend::touchpadHoldBegin(InputDevice *sender, uint8_t fingers)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    m_fingers = fingers;
    LibevdevComplementaryInputBackend::poll(); // Update clicked state
    m_block = handleEvent(TouchpadGestureLifecyclePhaseEvent(sender, TouchpadGestureLifecyclePhase::Begin, TriggerType::Press, fingers));
    return m_block;
}

bool LibinputInputBackend::touchpadHoldEnd(InputDevice *sender, bool cancelled)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    LibevdevComplementaryInputBackend::poll(); // Update clicked state
    handleEvent(TouchpadGestureLifecyclePhaseEvent(sender,
                                                   cancelled ? TouchpadGestureLifecyclePhase::Cancel : TouchpadGestureLifecyclePhase::End,
                                                   TriggerType::Press));
    return m_block;
}

bool LibinputInputBackend::touchpadPinchBegin(InputDevice *sender, uint8_t fingers)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    m_fingers = fingers;
    m_block = handleEvent(TouchpadGestureLifecyclePhaseEvent(sender, TouchpadGestureLifecyclePhase::Begin, TriggerType::PinchRotate, fingers));
    return m_block;
}

bool LibinputInputBackend::touchpadPinchUpdate(InputDevice *sender, qreal scale, qreal angleDelta)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    const auto block = handleEvent(TouchpadPinchEvent(sender, scale, angleDelta));
    if (m_block && !block) {
        // Allow the compositor/client to handle the gesture
        g_inputEmitter->touchpadPinchBegin(m_fingers);
    }
    m_block = block;
    return block;
}

bool LibinputInputBackend::touchpadPinchEnd(InputDevice *sender, bool cancelled)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    return handleEvent(TouchpadGestureLifecyclePhaseEvent(sender,
                                                          cancelled ? TouchpadGestureLifecyclePhase::Cancel : TouchpadGestureLifecyclePhase::End,
                                                          TriggerType::PinchRotate));
}

bool LibinputInputBackend::touchpadSwipeBegin(InputDevice *sender, uint8_t fingers)
{
    if (m_ignoreEvents || !sender) {
        return false;
    } else if (m_isRecordingStroke) {
        return true;
    }

    m_fingers = fingers;
    m_block = handleEvent(TouchpadGestureLifecyclePhaseEvent(sender, TouchpadGestureLifecyclePhase::Begin, TriggerType::StrokeSwipe, fingers));
    return m_block;
}

bool LibinputInputBackend::touchpadSwipeUpdate(InputDevice *sender, const PointDelta &delta)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    if (m_isRecordingStroke) {
        m_strokePoints.push_back(delta.unaccelerated());
        return true;
    }

    const auto block = handleEvent(MotionEvent(sender, InputEventType::TouchpadSwipe, delta));
    if (m_block && !block) {
        // Allow the compositor/client to handle the gesture
        g_inputEmitter->touchpadSwipeBegin(m_fingers);
    }
    m_block = block;
    return block;
}

bool LibinputInputBackend::touchpadSwipeEnd(InputDevice *sender, bool cancelled)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    if (m_isRecordingStroke) {
        finishStrokeRecording();
        return true;
    }

    return handleEvent(TouchpadGestureLifecyclePhaseEvent(sender,
                                                          cancelled ? TouchpadGestureLifecyclePhase::Cancel : TouchpadGestureLifecyclePhase::End,
                                                          TriggerType::StrokeSwipe));
}

Qt::MouseButton LibinputInputBackend::scanCodeToMouseButton(uint32_t scanCode) const
{
    static const std::map<uint32_t, Qt::MouseButton> buttons = {
        {BTN_LEFT, Qt::LeftButton},
        {BTN_MIDDLE, Qt::MiddleButton},
        {BTN_RIGHT, Qt::RightButton},
        // in QtWayland mapped like that
        {BTN_SIDE, Qt::ExtraButton1},
        {BTN_EXTRA, Qt::ExtraButton2},
        {BTN_FORWARD, Qt::ExtraButton3},
        {BTN_BACK, Qt::ExtraButton4},
        {BTN_TASK, Qt::ExtraButton5},
        {0x118, Qt::ExtraButton6},
        {0x119, Qt::ExtraButton7},
        {0x11a, Qt::ExtraButton8},
        {0x11b, Qt::ExtraButton9},
        {0x11c, Qt::ExtraButton10},
        {0x11d, Qt::ExtraButton11},
        {0x11e, Qt::ExtraButton12},
        {0x11f, Qt::ExtraButton13},
    };
    return buttons.at(scanCode);
}

}