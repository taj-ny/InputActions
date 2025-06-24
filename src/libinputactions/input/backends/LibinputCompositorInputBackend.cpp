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

#include "LibinputCompositorInputBackend.h"

#include <libinputactions/interfaces/InputEmitter.h>

namespace libinputactions
{

static const uint32_t STROKE_RECORD_TIMEOUT = 250;

bool LibinputCompositorInputBackend::keyboardKey(InputDevice *sender, const uint32_t &key, const bool &state)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    const KeyboardKeyEvent keyEvent(sender, key, state);
    handleEvent(&keyEvent);
    return false;
}

bool LibinputCompositorInputBackend::pointerAxis(InputDevice *sender, const QPointF &delta)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    if (sender->type() == InputDeviceType::Mouse) {
        const MotionEvent wheelEvent(sender, InputEventType::PointerScroll, delta);
        return handleEvent(&wheelEvent);
    }

    if (m_isRecordingStroke) {
        if (delta.isNull()) {
            finishStrokeRecording();
        } else {
            m_strokePoints.push_back(delta);
        }
        return true;
    }

    LibevdevComplementaryInputBackend::poll(); // Update finger count
    const MotionEvent scrollEvent(sender, InputEventType::PointerScroll, delta);
    return handleEvent(&scrollEvent);
}

bool LibinputCompositorInputBackend::pointerButton(InputDevice *sender, const Qt::MouseButton &button, const quint32 &nativeButton, const bool &state)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    if (sender->type() == InputDeviceType::Touchpad) {
        LibevdevComplementaryInputBackend::poll(); // Update clicked state
    }
    const PointerButtonEvent buttonEvent(sender, button, nativeButton, state);
    return handleEvent(&buttonEvent);
}

bool LibinputCompositorInputBackend::pointerMotion(InputDevice *sender, const QPointF &delta)
{
    if (m_ignoreEvents || !sender || sender->type() != InputDeviceType::Mouse) {
        return false;
    }

    if (m_isRecordingStroke) {
        m_strokePoints.push_back(delta);
        m_strokeRecordingTimeoutTimer.start(STROKE_RECORD_TIMEOUT);
    } else {
        const MotionEvent motionEvent(sender, InputEventType::PointerMotion, delta);
        handleEvent(&motionEvent);
    }
    return false;
}

bool LibinputCompositorInputBackend::touchpadHoldBegin(InputDevice *sender, const uint8_t &fingers)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    m_fingers = fingers;
    LibevdevComplementaryInputBackend::poll(); // Update clicked state
    const TouchpadGestureLifecyclePhaseEvent event(sender, TouchpadGestureLifecyclePhase::Begin, TriggerType::Press, fingers);
    m_block = handleEvent(&event);
    return m_block;
}

bool LibinputCompositorInputBackend::touchpadHoldEnd(InputDevice *sender, const bool &cancelled)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    LibevdevComplementaryInputBackend::poll(); // Update clicked state
    const TouchpadGestureLifecyclePhaseEvent event(sender, cancelled ? TouchpadGestureLifecyclePhase::Cancel : TouchpadGestureLifecyclePhase::End,
        TriggerType::Press);
    handleEvent(&event);
    return m_block;
}

bool LibinputCompositorInputBackend::touchpadPinchBegin(InputDevice *sender, const uint8_t &fingers)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    m_fingers = fingers;
    LibevdevComplementaryInputBackend::poll(); // Update finger count
    const TouchpadGestureLifecyclePhaseEvent event(sender, TouchpadGestureLifecyclePhase::Begin, TriggerType::PinchRotate, fingers);
    m_block = handleEvent(&event);
    return m_block;
}

bool LibinputCompositorInputBackend::touchpadPinchUpdate(InputDevice *sender, const qreal &scale, const qreal &angleDelta)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    const TouchpadPinchEvent event(sender, scale, angleDelta);
    const auto block = handleEvent(&event);
    if (m_block && !block) {
        // Allow the compositor/client to handle the gesture
        InputEmitter::instance()->touchpadPinchBegin(m_fingers);
    }
    m_block = block;
    return block;
}

bool LibinputCompositorInputBackend::touchpadPinchEnd(InputDevice *sender, const bool &cancelled)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    const TouchpadGestureLifecyclePhaseEvent event(sender, cancelled ? TouchpadGestureLifecyclePhase::Cancel : TouchpadGestureLifecyclePhase::End,
        TriggerType::PinchRotate);
    return handleEvent(&event);
}

bool LibinputCompositorInputBackend::touchpadSwipeBegin(InputDevice *sender, const uint8_t &fingers)
{
    if (m_ignoreEvents || !sender) {
        return false;
    } else if (m_isRecordingStroke) {
        return true;
    }

    m_fingers = fingers;
    LibevdevComplementaryInputBackend::poll(); // Update finger count
    const TouchpadGestureLifecyclePhaseEvent event(sender, TouchpadGestureLifecyclePhase::Begin, TriggerType::StrokeSwipe, fingers);
    m_block = handleEvent(&event);
    return m_block;
}

bool LibinputCompositorInputBackend::touchpadSwipeUpdate(InputDevice *sender, const QPointF &delta)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    if (m_isRecordingStroke) {
        m_strokePoints.push_back(delta);
        return true;
    }

    const MotionEvent event(sender, InputEventType::TouchpadSwipe, delta);
    const auto block = handleEvent(&event);
    if (m_block && !block) {
        // Allow the compositor/client to handle the gesture
        InputEmitter::instance()->touchpadSwipeBegin(m_fingers);
    }
    m_block = block;
    return block;
}

bool LibinputCompositorInputBackend::touchpadSwipeEnd(InputDevice *sender, const bool &cancelled)
{
    if (m_ignoreEvents || !sender) {
        return false;
    }

    if (m_isRecordingStroke) {
        finishStrokeRecording();
        return true;
    }

    const TouchpadGestureLifecyclePhaseEvent event(sender, cancelled ? TouchpadGestureLifecyclePhase::Cancel : TouchpadGestureLifecyclePhase::End,
        TriggerType::StrokeSwipe);
    return handleEvent(&event);
}

Qt::MouseButton LibinputCompositorInputBackend::scanCodeToMouseButton(const uint32_t &scanCode) const
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