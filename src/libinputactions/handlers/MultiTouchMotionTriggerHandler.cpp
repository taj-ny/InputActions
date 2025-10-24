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

#include "MultiTouchMotionTriggerHandler.h"
#include <libinputactions/input/events.h>
#include <libinputactions/variables/VariableManager.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_HANDLER_MULTITOUCH, "inputactions.handler.multitouch", QtWarningMsg)

namespace libinputactions
{

bool MultiTouchMotionTriggerHandler::touchChanged(const TouchChangedEvent &event)
{
    switch (m_state) {
        case State::LibinputTapBegin:
            return false;
        case State::Touch:
        case State::TouchIdle:
            const auto diff = event.point().position - event.point().initialPosition;
            if (std::hypot(diff.x(), diff.y()) >= 0.02) {
                setState(State::Motion);
            }
            break;
    }

    updateVariables(event.sender());
    return false;
}

bool MultiTouchMotionTriggerHandler::touchDown(const TouchEvent &event)
{
    switch (m_state) {
        case State::LibinputTapBegin:
            setState(State::TouchIdle);
            break;
        case State::None:
            setState(State::TouchIdle);
            m_firstTouchPoint = event.point();
            break;
    }

    updateVariables(event.sender());
    return false;
}

bool MultiTouchMotionTriggerHandler::touchUp(const TouchEvent &event)
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
                if (m_state == State::TapBegin && event.sender()->validTouchPoints().empty()) {
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
    if (event.sender()->validTouchPoints().empty()) {
        setState(State::None);
        endTriggers(TriggerType::All);
    }

    return false;
}

bool MultiTouchMotionTriggerHandler::handlePinch(qreal scale, qreal angleDelta)
{
    if (!hasActiveTriggers(TriggerType::PinchRotate)) {
        return false;
    }

    qCDebug(INPUTACTIONS_HANDLER_MULTITOUCH).nospace() << "Pinch event (scale: " << scale << ", angleDelta: " << angleDelta << ", delta: " << ")";

    const auto scaleDelta = -(m_previousPinchScale - scale);
    m_previousPinchScale = scale;

    m_accumulatedRotateDelta += std::abs(angleDelta);
    if (m_pinchType == PinchType::Unknown) {
        if (m_accumulatedRotateDelta >= 10) {
            qCDebug(INPUTACTIONS_HANDLER_MULTITOUCH, "Determined pinch type (rotate)");
            m_pinchType = PinchType::Rotate;
            cancelTriggers(TriggerType::Pinch);
        } else if (std::abs(1.0 - scale) >= 0.2) {
            qCDebug(INPUTACTIONS_HANDLER_MULTITOUCH, "Determined pinch type (pinch)");
            m_pinchType = PinchType::Pinch;
            cancelTriggers(TriggerType::Rotate);
        } else {
            qCDebug(INPUTACTIONS_HANDLER_MULTITOUCH, "Event processed (type: Pinch, status: DeterminingType)");
            return true;
        }
    }

    TriggerType type{};
    uint32_t direction{};
    qreal delta{};

    switch (m_pinchType) {
        case PinchType::Pinch:
            direction = static_cast<TriggerDirection>(scale < 1 ? PinchDirection::In : PinchDirection::Out);
            delta = scaleDelta;
            type = TriggerType::Pinch;
            break;
        case PinchType::Rotate:
            direction = static_cast<TriggerDirection>(angleDelta > 0 ? RotateDirection::Clockwise : RotateDirection::Counterclockwise);
            delta = angleDelta;
            type = TriggerType::Rotate;
            break;
        case PinchType::Unknown:
            break;
    }

    TriggerSpeed speed{};
    if (!determineSpeed(type, delta, speed, direction)) {
        qCDebug(INPUTACTIONS_HANDLER_MULTITOUCH, "Event processed (type: Pinch, status: DeterminingSpeed)");
        return true;
    }

    DirectionalMotionTriggerUpdateEvent event;
    event.m_delta = delta;
    event.m_direction = direction;
    event.m_speed = speed;
    const auto result = updateTriggers(type, event);

    qCDebug(INPUTACTIONS_HANDLER_MULTITOUCH).nospace() << "Event processed (type: Pinch, hasGestures: " << result.success << ")";
    return result.block;
}

void MultiTouchMotionTriggerHandler::reset()
{
    MotionTriggerHandler::reset();
    m_previousPinchScale = 1;
    m_pinchType = PinchType::Unknown;
    m_accumulatedRotateDelta = 0;
}

void MultiTouchMotionTriggerHandler::updateVariables(const InputDevice *sender)
{
    auto thumbInitialPosition = g_variableManager->getVariable(BuiltinVariables::ThumbInitialPositionPercentage);
    auto thumbPosition = g_variableManager->getVariable(BuiltinVariables::ThumbPositionPercentage);
    auto thumbPresent = g_variableManager->getVariable(BuiltinVariables::ThumbPresent);
    bool hasThumb{};

    const auto touchPoints = sender ? sender->validTouchPoints() : std::vector<const TouchPoint *>();
    for (size_t i = 0; i < s_fingerVariableCount; i++) {
        const auto fingerVariableNumber = i + 1;
        auto initialPosition = g_variableManager->getVariable<QPointF>(QString("finger_%1_initial_position_percentage").arg(fingerVariableNumber));
        auto position = g_variableManager->getVariable<QPointF>(QString("finger_%1_position_percentage").arg(fingerVariableNumber));
        auto pressure = g_variableManager->getVariable<qreal>(QString("finger_%1_pressure").arg(fingerVariableNumber));

        if (touchPoints.size() <= i || !touchPoints[i]->valid) {
            initialPosition->set({});
            position->set({});
            pressure->set({});
            continue;
        }

        const auto *point = touchPoints[i];
        if (point->type == TouchPointType::Thumb) {
            hasThumb = true;
            thumbInitialPosition->set(point->initialPosition);
            thumbPosition->set(point->position);
            thumbPresent->set(true);
        }
        initialPosition->set(point->initialPosition);
        position->set(point->position);
        pressure->set(point->pressure);
    }

    if (!hasThumb) {
        thumbInitialPosition->set({});
        thumbPosition->set({});
        thumbPresent->set(false);
    }

    g_variableManager->getVariable(BuiltinVariables::Fingers)->set(touchPoints.size());
}

void MultiTouchMotionTriggerHandler::setState(State state)
{
    m_state = state;
    switch (state) {
        case State::None:
            updateVariables();
            break;
    }
}

bool MultiTouchMotionTriggerHandler::canTap()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_firstTouchPoint.downTimestamp).count()
        <= TAP_TIMEOUT.count();
}

}