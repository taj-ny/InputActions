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

namespace InputActions
{

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
    event.setDelta(delta);
    event.setDirection(direction);
    event.setSpeed(speed);
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

        if (!sender || touchPoints.size() <= i || !touchPoints[i]->valid) {
            initialPosition->set({});
            position->set({});
            pressure->set({});
            continue;
        }

        const auto *point = touchPoints[i];
        if (point->type == TouchPointType::Thumb) {
            hasThumb = true;
            thumbInitialPosition->set(point->initialPosition / sender->properties().size());
            thumbPosition->set(point->position / sender->properties().size());
            thumbPresent->set(true);
        }
        initialPosition->set(point->initialPosition / sender->properties().size());
        position->set(point->position / sender->properties().size());
        pressure->set(point->pressure);
    }

    if (!hasThumb) {
        thumbInitialPosition->set({});
        thumbPosition->set({});
        thumbPresent->set(false);
    }

    g_variableManager->getVariable(BuiltinVariables::Fingers)->set(touchPoints.size());
}

}