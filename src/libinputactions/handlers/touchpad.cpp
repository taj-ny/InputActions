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
        case InputEventType::PointerButton:
            if (!(event->sender()->types() & InputDeviceType::Touchpad)) {
                return false;
            }
            return handleEvent(static_cast<const PointerButtonEvent *>(event));
        case InputEventType::PointerScroll:
            if (!(event->sender()->types() & InputDeviceType::Touchpad)) {
                return false;
            }
            return handleScrollEvent(static_cast<const MotionEvent *>(event));
        case InputEventType::TouchpadClick:
            return handleEvent(static_cast<const TouchpadClickEvent *>(event));
        case InputEventType::TouchpadGestureLifecyclePhase:
            return handleEvent(static_cast<const TouchpadGestureLifecyclePhaseEvent *>(event));
        case InputEventType::TouchpadPinch:
            return handleEvent(static_cast<const TouchpadPinchEvent *>(event));
        case InputEventType::TouchpadSlot:
            return handleEvent(static_cast<const TouchpadSlotEvent *>(event));
        case InputEventType::TouchpadSwipe:
            return handleSwipeEvent(static_cast<const MotionEvent *>(event));
        default:
            return false;
    }
}

bool TouchpadTriggerHandler::handleEvent(const PointerButtonEvent *event)
{
    if (event->state() && m_clicked) {
        cancelTriggers(TriggerType::Press);
        return activateTriggers(TriggerType::Click);
    } else if (!event->state() && !m_clicked) {
        return endTriggers(TriggerType::Click);
    }
    return false;
}

bool TouchpadTriggerHandler::handleEvent(const TouchpadClickEvent *event)
{
    // Activation is done in PointerButtonEvent handler
    m_clicked = event->state();
    return false;
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
            // Libinput ends hold gestures when the touchpad is clicked instead of cancelling
            if (m_clicked && event->triggers() == TriggerType::Press) {
                return cancelTriggers(event->triggers());
            }
            return endTriggers(event->triggers());
        default:
            return false;
    }
}

bool TouchpadTriggerHandler::handleEvent(const TouchpadPinchEvent *event)
{
    return handlePinch(event->scale(), event->angleDelta());
}

bool TouchpadTriggerHandler::handleEvent(const TouchpadSlotEvent *event)
{
    m_usesLibevdevBackend = true;

    auto *manager = VariableManager::instance();
    auto thumbPresent = manager->getVariable(BuiltinVariables::ThumbPresent);
    auto thumbPosition = manager->getVariable(BuiltinVariables::ThumbPositionPercentage);
    bool hasThumb{};

    for (auto i = 0; i < std::min(static_cast<uint8_t>(event->fingerSlots().size()), s_fingerVariableCount); i++) {
        const auto &slot = event->fingerSlots()[i];
        const auto fingerVariableNumber = i + 1;

        auto position = manager->getVariable<QPointF>(QString("finger_%1_position_percentage").arg(fingerVariableNumber));
        auto pressure = manager->getVariable<qreal>(QString("finger_%1_pressure").arg(fingerVariableNumber));

        if (!slot.active) {
            position->set({});
            pressure->set({});
            continue;
        }

        if (event->sender()->properties().thumbPressureRange().contains(slot.pressure)) {
            hasThumb = true;
            thumbPresent->set(true);
            thumbPosition->set(slot.position);
        }
        position->set(slot.position);
        pressure->set(slot.pressure);
    }

    if (!hasThumb) {
        thumbPresent->set(false);
        thumbPosition->set({});
    }
    return false;
}

bool TouchpadTriggerHandler::handleScrollEvent(const MotionEvent *event)
{
    if (event->delta().isNull()) {
        endTriggers(TriggerType::StrokeSwipe);
        m_scrollInProgress = false;
        return false; // Blocking a (0,0) event breaks kinetic scrolling
    }

    if (!m_scrollInProgress) {
        if (!m_usesLibevdevBackend) {
            VariableManager::instance()->getVariable(BuiltinVariables::Fingers)->set(2);
        }
        m_scrollInProgress = true;
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