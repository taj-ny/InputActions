/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

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
#include <libinputactions/helpers/Math.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/input/devices/InputDeviceProperties.h>
#include <libinputactions/input/events.h>
#include <libinputactions/variables/VariableManager.h>
#include <linux/input-event-codes.h>

namespace InputActions
{

static const std::chrono::milliseconds TAP_TIMEOUT(200L);
static const std::chrono::milliseconds LIBINPUT_TAP_TIMEOUT(300L);

TouchpadTriggerHandler::TouchpadTriggerHandler(InputDevice *device)
{
    setDevice(device);

    m_clickTimeoutTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_clickTimeoutTimer.setSingleShot(true);

    m_libinputTapTimeoutTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_libinputTapTimeoutTimer.setSingleShot(true);
    connect(&m_libinputTapTimeoutTimer, &QTimer::timeout, this, &TouchpadTriggerHandler::onLibinputTapTimeout);
}

bool TouchpadTriggerHandler::pointerAxis(const MotionEvent &event)
{
    bool isFirstEvent{};
    switch (m_state) {
        case State::Motion:
        case State::MotionNoTrigger:
        case State::None:
        case State::Touch:
        case State::TouchIdle:
            isFirstEvent = true;
            g_variableManager->getVariable(BuiltinVariables::Fingers)->set(2);
            setState(State::Scrolling);
            activateTriggers(TriggerType::SinglePointMotion);
            [[fallthrough]];
        case State::Scrolling: {
            if (event.delta().unaccelerated().isNull()) {
                endTriggers(TriggerType::SinglePointMotion);
                setState(State::None);

                m_previousPointerAxisEventBlocked = false;
                m_pointerAxisDelta = {};

                return false; // Blocking a (0,0) event breaks kinetic scrolling
            }

            std::vector<PointDelta> deltas;
            if (isFirstEvent || !event.oneAxisPerEvent()) {
                // First event must always be passed through for blocking
                deltas.push_back(event.delta());
            } else if (m_pointerAxisDelta.unaccelerated().isNull()) {
                m_pointerAxisDelta = event.delta();
                return m_previousPointerAxisEventBlocked;
            } else {
                const auto sum = m_pointerAxisDelta + event.delta();
                if (sum.unaccelerated().x() && sum.unaccelerated().y()) {
                    deltas.push_back(sum);
                } else {
                    // Don't merge if both events have the same one axis
                    deltas.push_back(m_pointerAxisDelta);
                    deltas.push_back(event.delta());
                }
            }
            m_pointerAxisDelta = {};

            bool block{};
            for (const auto &delta : deltas) {
                block = handleMotion(event.sender(), delta) || block;
            }
            m_previousPointerAxisEventBlocked = block;
            return block;
        }
        default:
            return false;
    }
}

bool TouchpadTriggerHandler::pointerButton(const PointerButtonEvent &event)
{
    bool block{};
    switch (m_state) {
        case State::LibinputTapBegin:
        case State::TouchIdle:
            if (event.state() && event.sender()->physicalState().validTouchPoints().size() <= 3) {
                uint8_t fingers;
                if (event.button().scanCode() == BTN_LEFT) {
                    fingers = 1;
                } else if (event.button().scanCode() == BTN_RIGHT) {
                    fingers = event.sender()->properties().touchpadLmrTapButtonMap() ? 3 : 2;
                } else if (event.button().scanCode() == BTN_MIDDLE) {
                    fingers = event.sender()->properties().touchpadLmrTapButtonMap() ? 2 : 3;
                } else {
                    break;
                }
                g_variableManager->getVariable(BuiltinVariables::Fingers)->set(fingers);
                if (const auto result = activateTriggers(TriggerType::Tap); result.success) {
                    updateTriggers(TriggerType::Tap);
                    endTriggers(TriggerType::Tap);
                    block = result.block;
                }
                updateVariables(event.sender());
                setState(State::None);
            }
            break;
        case State::TouchpadButtonDownBlocked:
            block = true;
            break;
    }

    if (block && event.state()) {
        m_blockedButtons.insert(event.button());
        return true;
    } else if (!event.state()) {
        return m_blockedButtons.erase(event.button()) || block;
    }
    return false;
}

bool TouchpadTriggerHandler::pointerMotion(const MotionEvent &event)
{
    switch (m_state) {
        case State::Motion:
        case State::None:
        case State::Touch:
        case State::TouchIdle:
            g_variableManager->getVariable(BuiltinVariables::Fingers)->set(1);
            setState(activateTriggers(TriggerType::SinglePointMotion).success ? State::MotionTrigger : State::MotionNoTrigger);
            [[fallthrough]];
        case State::MotionTrigger:
            return handleMotion(event.sender(), event.delta());
    }
    return false;
}

bool TouchpadTriggerHandler::touchDown(const TouchDownEvent &event)
{
    switch (m_state) {
        case State::LibinputTapBegin:
            setState(State::TouchIdle);
            break;
        case State::None:
            setState(State::TouchIdle);
            m_firstTouchPoint = *event.sender()->physicalState().findTouchPoint(event.id());
            break;
    }

    updateVariables(event.sender());
    return false;
}

bool TouchpadTriggerHandler::touchMotion(const TouchMotionEvent &event)
{
    switch (m_state) {
        case State::LibinputTapBegin:
            return false;
        case State::Touch:
        case State::TouchIdle:
            const auto *point = event.sender()->physicalState().findTouchPoint(event.id());
            const auto diff = point->position - point->initialPosition;
            if (Math::hypot(diff) >= 4) {
                setState(State::Motion);
            }
            break;
    }

    updateVariables(event.sender());
    return false;
}

bool TouchpadTriggerHandler::touchUp(const TouchUpEvent &event)
{
    switch (m_state) {
        case State::TapBegin:
        case State::TouchIdle:
            // 1-3 finger touchpad tap gestures are detected by listening for pointer button events, as it's more reliable. The child class should reset the
            // state in case no pointer button events occur.
            if (m_state == State::TouchIdle && event.sender()->type() == InputDeviceType::Touchpad
                && g_variableManager->getVariable(BuiltinVariables::Fingers)->get() <= 3) {
                setState(State::LibinputTapBegin);
                break;
            }

            if (canTap()) {
                if (m_state == State::TouchIdle) {
                    setState(activateTriggers(TriggerType::Tap).success ? State::TapBegin : State::Touch);
                }
                if (m_state == State::TapBegin && event.sender()->physicalState().validTouchPoints().empty()) {
                    updateTriggers(TriggerType::Tap);
                    endTriggers(TriggerType::Tap);
                    setState(State::None);
                }
                break;
            }
            if (m_state == State::TapBegin) {
                cancelTriggers(TriggerType::Tap);
            }
            setState(State::Touch);
            break;
    }

    if (m_state == State::LibinputTapBegin) {
        return false;
    }

    updateVariables(event.sender());
    if (event.sender()->physicalState().validTouchPoints().empty()) {
        setState(State::None);
        endTriggers(TriggerType::All);
    }

    return false;
}

bool TouchpadTriggerHandler::touchpadClick(const TouchpadClickEvent &event)
{
    if (event.state()) {
        cancelTriggers(TriggerType::Press);
        setState(activateTriggers(TriggerType::Click).block ? State::TouchpadButtonDownBlocked : State::TouchpadButtonDown);
    } else if (m_state == State::TouchpadButtonDown || m_state == State::TouchpadButtonDownBlocked) {
        setState(event.sender()->physicalState().validTouchPoints().empty() ? State::None : State::Touch);
        endTriggers(TriggerType::Click);
    }

    m_clickTimeoutTimer.stop();
    return false;
}

bool TouchpadTriggerHandler::touchpadGestureLifecyclePhase(const TouchpadGestureLifecyclePhaseEvent &event)
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
                m_clickTimeoutTimer.start(std::max(TAP_TIMEOUT.count(), m_device->properties().touchpadClickTimeout().count()));
                return m_gestureBeginBlocked;
            }

            return activateTriggers(event.triggers()).block && m_gestureBeginBlocked;
        }
        case TouchpadGestureLifecyclePhase::Cancel:
            m_clickTimeoutTimer.stop();
            return cancelTriggers(event.triggers()).block && m_gestureBeginBlocked;
        case TouchpadGestureLifecyclePhase::End:
            m_clickTimeoutTimer.stop();
            // Libinput ends hold gestures when the touchpad is clicked instead of cancelling
            if ((m_state == State::TouchpadButtonDown || m_state == State::TouchpadButtonDownBlocked) && event.triggers() == TriggerType::Press) {
                return cancelTriggers(event.triggers()).block && m_gestureBeginBlocked;
            }
            return endTriggers(event.triggers()).block && m_gestureBeginBlocked;
        default:
            return false;
    }
}

bool TouchpadTriggerHandler::touchpadPinch(const TouchpadPinchEvent &event)
{
    return handlePinch(event.scale(), event.angleDelta());
}

bool TouchpadTriggerHandler::touchpadSwipe(const MotionEvent &event)
{
    return handleMotion(event.sender(), event.delta());
}

bool TouchpadTriggerHandler::canTap()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_firstTouchPoint.downTimestamp).count()
        <= TAP_TIMEOUT.count();
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
        case State::None:
            updateVariables();
            break;
    }

    m_state = state;
}

void TouchpadTriggerHandler::onLibinputTapTimeout()
{
    if (m_state == State::LibinputTapBegin) {
        setState(State::None);
    }
}

}