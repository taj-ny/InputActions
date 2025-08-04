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

namespace libinputactions
{

TouchpadTriggerHandler::TouchpadTriggerHandler()
{
    m_clickTimeoutTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_clickTimeoutTimer.setSingleShot(true);
}

bool TouchpadTriggerHandler::handleEvent(const InputEvent &event)
{
    if (MultiTouchMotionTriggerHandler::handleEvent(event)) {
        return true;
    }

    switch (event.type()) {
        case InputEventType::PointerButton:
            if (event.sender()->type() != InputDeviceType::Touchpad) {
                return false;
            }
            return handleEvent(static_cast<const PointerButtonEvent &>(event));
        case InputEventType::PointerScroll:
            if (event.sender()->type() != InputDeviceType::Touchpad) {
                return false;
            }
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

void TouchpadTriggerHandler::setClickTimeout(uint32_t value)
{
    m_clickTimeout = value;
}

bool TouchpadTriggerHandler::handleEvent(const PointerButtonEvent &event)
{
    switch (m_state) {
        case State::TouchpadButtonDown:
            return true;
        case State::TapEnd:
            if (!event.state()) {
                m_state = State::None;
            }
            return true;
        case State::TouchpadButtonUp:
            m_state = m_savedState;
            if (m_state == State::TouchIdle) {
                m_state = State::Touch;
            }
            return true;
        default:
            return false;
    }
}

void TouchpadTriggerHandler::handleEvent(const TouchpadClickEvent &event)
{
    if (event.state()) {
        cancelTriggers(TriggerType::Press);
        if (activateTriggers(TriggerType::Click)) {
            m_savedState = m_state;
            m_state = State::TouchpadButtonDown;
        }
    } else if (m_state == State::TouchpadButtonDown) {
        m_state = State::TouchpadButtonUp;
        endTriggers(TriggerType::Click);
    }

    m_clickTimeoutTimer.stop();
}

bool TouchpadTriggerHandler::handleEvent(const TouchpadGestureLifecyclePhaseEvent &event)
{
    switch (event.phase()) {
        case TouchpadGestureLifecyclePhase::Begin:
            g_variableManager->getVariable(BuiltinVariables::Fingers)->set(event.fingers());
            {
                // Delay press gesture activation if there is a click gesture
                TriggerActivationEvent activationEvent;
                if (event.triggers() & TriggerType::Press && !triggers(TriggerType::Click, activationEvent).empty()) {
                    const auto triggers = event.triggers();
                    QObject::disconnect(&m_clickTimeoutTimer, nullptr, nullptr, nullptr);
                    QObject::connect(&m_clickTimeoutTimer, &QTimer::timeout, [triggers, this] {
                        if (hasActiveTriggers(TriggerType::All & ~triggers)) {
                            return;
                        }
                        activateTriggers(triggers);
                    });
                    m_clickTimeoutTimer.start(m_clickTimeout);
                    return true;
                }
            }
            return activateTriggers(event.triggers());
        case TouchpadGestureLifecyclePhase::Cancel:
            m_clickTimeoutTimer.stop();
            return cancelTriggers(event.triggers());
        case TouchpadGestureLifecyclePhase::End:
            m_clickTimeoutTimer.stop();
            if (m_state == State::TouchpadButtonDown && event.triggers() == TriggerType::Press) {
                return true;
            }
            return endTriggers(event.triggers());
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
        case State::None:
        case State::TouchIdle:
            if (!m_usesLibevdevBackend) {
                g_variableManager->getVariable(BuiltinVariables::Fingers)->set(2);
            }
            m_state = State::Scrolling;
            activateTriggers(TriggerType::StrokeSwipe);
            [[fallthrough]];
        case State::Scrolling:
            if (event.delta().isNull()) {
                endTriggers(TriggerType::StrokeSwipe);
                m_state = State::None;
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