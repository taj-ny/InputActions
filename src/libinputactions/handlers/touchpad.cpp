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

#include "touchpad.h"

#include <libinputactions/variables/manager.h>

namespace libinputactions
{

bool TouchpadTriggerHandler::handleEvent(const InputEvent *event)
{
    MultiTouchMotionTriggerHandler::handleEvent(event);
    switch (event->type()) {
        case InputEventType::TouchpadGestureLifecyclePhase:
            return handleEvent(static_cast<const TouchpadGestureLifecyclePhaseEvent *>(event));
        case InputEventType::TouchpadPinch:
            return handleEvent(static_cast<const TouchpadPinchEvent *>(event));
        case InputEventType::TouchpadScroll:
            return handleScrollEvent(static_cast<const MotionEvent *>(event));
        case InputEventType::TouchpadSwipe:
            return handleSwipeEvent(static_cast<const MotionEvent *>(event));
        default:
            return false;
    }
}

bool TouchpadTriggerHandler::handleEvent(const TouchpadGestureLifecyclePhaseEvent *event)
{
    switch (event->phase()) {
        case TouchpadGestureLifecyclePhase::Begin:
            VariableManager::instance()->getVariable(BuiltinVariables::Fingers)->set(event->fingers());
            return activateTriggers(event->triggers());
        case TouchpadGestureLifecyclePhase::Cancel:
            return cancelTriggers(event->triggers());
        case TouchpadGestureLifecyclePhase::End:
            return endTriggers(event->triggers());
        default:
            return false;
    }
}

bool TouchpadTriggerHandler::handleEvent(const TouchpadPinchEvent *event)
{
    return handlePinch(event->scale(), event->angleDelta());
}

bool TouchpadTriggerHandler::handleScrollEvent(const MotionEvent *event)
{
    if (event->delta().isNull()) {
        endTriggers(TriggerType::StrokeSwipe);
        m_scrollInProgress = false;
        return false; // Blocking a (0,0) event breaks kinetic scrolling
    }

    if (!m_scrollInProgress) {
        m_scrollInProgress = true;
        VariableManager::instance()->getVariable(BuiltinVariables::Fingers)->set(2);
        activateTriggers(TriggerType::StrokeSwipe);
    }
    if (handleMotion(event->delta())) {
        return true;
    }
    return false;
}

bool TouchpadTriggerHandler::handleSwipeEvent(const MotionEvent *event)
{
    return handleMotion(event->delta());
}

}