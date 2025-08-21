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
#include <libinputactions/variables/VariableManager.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_HANDLER_MULTITOUCH, "inputactions.handler.multitouch", QtWarningMsg)

namespace libinputactions
{

bool MultiTouchMotionTriggerHandler::handleEvent(const InputEvent &event)
{
    if (TriggerHandler::handleEvent(event)) {
        return true;
    }

    switch (event.type()) {
        case InputEventType::TouchDown:
            handleTouchDownEvent(static_cast<const TouchEvent &>(event));
            break;
        case InputEventType::TouchChanged:
            handleEvent(static_cast<const TouchChangedEvent &>(event));
            break;
        case InputEventType::TouchUp:
            handleTouchUpEvent(static_cast<const TouchEvent &>(event));
            break;
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
    const auto hasGestures = updateTriggers(type, event);

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

void MultiTouchMotionTriggerHandler::handleTouchDownEvent(const TouchEvent &event)
{
    switch (m_state) {
        case State::None:
        case State::TapEnd: // In case user taps again before libinput sends a button released event
            m_state = State::TouchIdle;
            break;
    }
    updateVariables(event.sender());
}

void MultiTouchMotionTriggerHandler::handleEvent(const TouchChangedEvent &event)
{
    switch (m_state) {
        case State::TouchIdle:
            const auto diff = event.point().position - event.point().initialPosition;
            if (std::hypot(diff.x(), diff.y()) >= 0.02) {
                m_state = State::Touch;
            }
            break;
    }
    updateVariables(event.sender());
}

void MultiTouchMotionTriggerHandler::handleTouchUpEvent(const TouchEvent &event)
{
    switch (m_state) {
        case State::TapBegin:
        case State::TouchIdle:
            // 1-3 finger touchpad tap gestures are detected by listening for pointer button events, as it's more reliable.
            if (m_state == State::TouchIdle && event.sender()->type() == InputDeviceType::Touchpad
                && g_variableManager->getVariable(BuiltinVariables::Fingers)->get() <= 3) {
                return;
            }

            if (canTap(event.sender())) {
                if (m_state == State::TouchIdle) {
                    m_state = activateTriggers(TriggerType::Tap) ? State::TapBegin : State::Touch;
                }
                if (m_state == State::TapBegin && !event.sender()->validTouchPoints()) {
                    updateTriggers(TriggerType::Tap);
                    endTriggers(TriggerType::Tap);
                    m_state = State::None;
                }
            } else if (m_state == State::TapBegin) {
                cancelTriggers(TriggerType::Tap);
                m_state = State::TouchIdle;
            }
            break;
    }
    updateVariables(event.sender());

    if (m_state != State::TapEnd && !event.sender()->validTouchPoints()) {
        m_state = State::None;
    }
}

bool MultiTouchMotionTriggerHandler::canTap(const InputDevice *device)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - device->m_touchPoints[0].downTimestamp).count() <= TAP_TIMEOUT.count();
}

void MultiTouchMotionTriggerHandler::updateVariables(const InputDevice *sender)
{
    auto thumbPresent = g_variableManager->getVariable(BuiltinVariables::ThumbPresent);
    auto thumbPosition = g_variableManager->getVariable(BuiltinVariables::ThumbPositionPercentage);
    bool hasThumb{};

    for (auto i = 0; i < std::min(static_cast<uint8_t>(sender->m_touchPoints.size()), s_fingerVariableCount); i++) {
        const auto &slot = sender->m_touchPoints[i];
        const auto fingerVariableNumber = i + 1;

        auto position = g_variableManager->getVariable<QPointF>(QString("finger_%1_position_percentage").arg(fingerVariableNumber));
        auto pressure = g_variableManager->getVariable<qreal>(QString("finger_%1_pressure").arg(fingerVariableNumber));

        if (!slot.valid) {
            position->set({});
            pressure->set({});
            continue;
        }

        if (slot.type == TouchPointType::Thumb) {
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

    g_variableManager->getVariable(BuiltinVariables::Fingers)->set(sender->validTouchPoints());
}

}