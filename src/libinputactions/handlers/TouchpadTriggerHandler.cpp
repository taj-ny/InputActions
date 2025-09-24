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

#include "TouchpadTriggerHandler.h"
#include <libinputactions/variables/VariableManager.h>
#include <linux/input-event-codes.h>

namespace libinputactions
{

static const std::chrono::milliseconds LIBINPUT_TAP_TIMEOUT(300L);

TouchpadTriggerHandler::TouchpadTriggerHandler()
{
    m_clickTimeoutTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_clickTimeoutTimer.setSingleShot(true);

    m_libinputTapTimeoutTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_libinputTapTimeoutTimer.setSingleShot(true);
    connect(&m_libinputTapTimeoutTimer, &QTimer::timeout, this, &TouchpadTriggerHandler::onLibinputTapTimeout);
}

void TouchpadTriggerHandler::setState(State state)
{
    switch (m_state) {
        case State::LibinputTapBegin:
            m_libinputTapTimeoutTimer.stop();
            break;
    }
    switch (state) {
        case State::LibinputTapBegin:
            m_libinputTapTimeoutTimer.start(LIBINPUT_TAP_TIMEOUT);
            break;
    }
    MultiTouchMotionTriggerHandler::setState(state);
}

void TouchpadTriggerHandler::onLibinputTapTimeout()
{
    if (m_state == State::LibinputTapBegin) {
        setState(State::None);
    }
}

bool TouchpadTriggerHandler::handleEvent(const InputEvent &event)
{
    if (MultiTouchMotionTriggerHandler::handleEvent(event)) {
        return true;
    } else if (event.sender()->type() != InputDeviceType::Touchpad) {
        return false;
    }

    switch (event.type()) {
        case InputEventType::PointerButton:
            return handleEvent(static_cast<const PointerButtonEvent &>(event));
        case InputEventType::PointerMotion:
            return handleEvent(static_cast<const MotionEvent &>(event));
        case InputEventType::PointerScroll:
            return handleScrollEvent(static_cast<const MotionEvent &>(event));
        case InputEventType::TouchpadClick:
            handleEvent(static_cast<const TouchpadClickEvent &>(event));
            break;
        case InputEventType::TouchpadGestureLifecyclePhase:
            return handleEvent(static_cast<const TouchpadGestureLifecyclePhaseEvent &>(event));
        case InputEventType::TouchpadPinch:
            return handleEvent(static_cast<const TouchpadPinchEvent &>(event));
        case InputEventType::TouchpadSwipe:
            return handleSwipeEvent(static_cast<const MotionEvent &>(event));
    }
    return false;
}

bool TouchpadTriggerHandler::handleEvent(const MotionEvent &event)
{
    switch (m_state) {
        case State::Motion:
        case State::None:
        case State::Touch:
        case State::TouchIdle:
            g_variableManager->getVariable(BuiltinVariables::Fingers)->set(1);
            setState(activateTriggers(TriggerType::StrokeSwipe) ? State::MotionTrigger : State::MotionNoTrigger);
            [[fallthrough]];
        case State::MotionTrigger:
            return handleMotion(event.delta());
    }
    return false;
}

bool TouchpadTriggerHandler::handleEvent(const PointerButtonEvent &event)
{
    bool block{};
    switch (m_state) {
        case State::LibinputTapBegin:
        case State::TouchIdle:
            if (event.state() && event.sender()->validTouchPoints().size() <= 3) {
                uint8_t fingers;
                if (event.nativeButton() == BTN_LEFT) {
                    fingers = 1;
                } else if (event.nativeButton() == BTN_RIGHT) {
                    fingers = event.sender()->properties().lmrTapButtonMap() ? 3 : 2;
                } else if (event.nativeButton() == BTN_MIDDLE) {
                    fingers = event.sender()->properties().lmrTapButtonMap() ? 2 : 3;
                } else {
                    break;
                }
                g_variableManager->getVariable(BuiltinVariables::Fingers)->set(fingers);
                if (activateTriggers(TriggerType::Tap)) {
                    updateTriggers(TriggerType::Tap);
                    endTriggers(TriggerType::Tap);
                    block = true;
                }
                updateVariables(event.sender());
                setState(State::None);
            }
            break;
        case State::TouchpadButtonDownClickTrigger:
            block = true;
            break;
    }

    if (block && event.state()) {
        m_blockedButtons.insert(event.nativeButton());
        return true;
    } else if (!event.state()) {
        return m_blockedButtons.erase(event.nativeButton()) || block;
    }
    return false;
}

void TouchpadTriggerHandler::handleEvent(const TouchpadClickEvent &event)
{
    if (event.state()) {
        cancelTriggers(TriggerType::Press);
        setState(activateTriggers(TriggerType::Click) ? State::TouchpadButtonDownClickTrigger : State::TouchpadButtonDown);
    } else if (m_state == State::TouchpadButtonDown || m_state == State::TouchpadButtonDownClickTrigger) {
        setState(event.sender()->validTouchPoints().empty() ? State::None : State::Touch);
        endTriggers(TriggerType::Click);
    }

    m_clickTimeoutTimer.stop();
}

bool TouchpadTriggerHandler::handleEvent(const TouchpadGestureLifecyclePhaseEvent &event)
{
    switch (event.phase()) {
        case TouchpadGestureLifecyclePhase::Begin: {
            g_variableManager->getVariable(BuiltinVariables::Fingers)->set(event.fingers());

            // 1- and 2-finger hold gestures have almost no delay and are used to stop kinetic scrolling, there's no reason to block them
            m_gestureBeginBlocked = !(event.triggers() & TriggerType::Press && event.fingers() <= 2);

            // Delay press trigger activation if there is a click or a tap trigger
            TriggerActivationEvent activationEvent;
            if ((event.triggers() & TriggerType::Press) && !triggers(TriggerType::Click | TriggerType::Tap, activationEvent).empty()) {
                const auto triggers = event.triggers();
                QObject::disconnect(&m_clickTimeoutTimer, nullptr, nullptr, nullptr);
                QObject::connect(&m_clickTimeoutTimer, &QTimer::timeout, [triggers, this] {
                    if (hasActiveTriggers(TriggerType::All & ~triggers)) {
                        return;
                    }
                    activateTriggers(triggers);
                });
                m_clickTimeoutTimer.start(std::max(TAP_TIMEOUT.count(), m_clickTimeout.count()));
                return m_gestureBeginBlocked;
            }

            return activateTriggers(event.triggers()) && m_gestureBeginBlocked;
        }
        case TouchpadGestureLifecyclePhase::Cancel:
            m_clickTimeoutTimer.stop();
            return cancelTriggers(event.triggers()) && m_gestureBeginBlocked;
        case TouchpadGestureLifecyclePhase::End:
            m_clickTimeoutTimer.stop();
            // Libinput ends hold gestures when the touchpad is clicked instead of cancelling
            if ((m_state == State::TouchpadButtonDown || m_state == State::TouchpadButtonDownClickTrigger) && event.triggers() == TriggerType::Press) {
                return cancelTriggers(event.triggers()) && m_gestureBeginBlocked;
            }
            return endTriggers(event.triggers()) && m_gestureBeginBlocked;
        default:
            return false;
    }
}

bool TouchpadTriggerHandler::handleEvent(const TouchpadPinchEvent &event)
{
    return handlePinch(event.scale(), event.angleDelta());
}

bool TouchpadTriggerHandler::handleScrollEvent(const MotionEvent &event)
{
    switch (m_state) {
        case State::Motion:
        case State::MotionNoTrigger:
        case State::None:
        case State::Touch:
        case State::TouchIdle:
            g_variableManager->getVariable(BuiltinVariables::Fingers)->set(2);
            setState(State::Scrolling);
            activateTriggers(TriggerType::StrokeSwipe);
            [[fallthrough]];
        case State::Scrolling:
            if (event.delta().isNull()) {
                endTriggers(TriggerType::StrokeSwipe);
                setState(State::None);
                return false; // Blocking a (0,0) event breaks kinetic scrolling
            }

            return handleMotion(event.delta());
        default:
            return false;
    }
}

bool TouchpadTriggerHandler::handleSwipeEvent(const MotionEvent &event)
{
    return handleMotion(event.delta());
}

}