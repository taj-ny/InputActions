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

Q_LOGGING_CATEGORY(INPUTACTIONS_HANDLER_MULTITOUCH, "inputactions.handler.multitouch", QtWarningMsg)

namespace libinputactions
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
    event.m_delta = delta;
    event.m_direction = direction;
    event.m_speed = speed;
    const auto hasGestures = updateTriggers(type, &event);

    qCDebug(INPUTACTIONS_HANDLER_MULTITOUCH).nospace() << "Event processed (type: Pinch, hasGestures: " << hasGestures << ")";
    return hasGestures;
}

void MultiTouchMotionTriggerHandler::reset()
{
    MotionTriggerHandler::reset();
    m_previousPinchScale = 1;
    m_pinchType = PinchType::Unknown;
    m_accumulatedRotateDelta = 0;
}

}