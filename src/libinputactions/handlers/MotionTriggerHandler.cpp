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


    Code for circle triggers was taken from https://github.com/galundin/circular-scrolling-improved

    Copyright 2013 Andrew Lundin
    MIT License
    http://opensource.org/licenses/MIT
*/

#include "MotionTriggerHandler.h"
#include <libinputactions/helpers/Math.h>
#include <libinputactions/input/Delta.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/triggers/StrokeTrigger.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_HANDLER_MOTION, "inputactions.handler.motion", QtWarningMsg)

namespace InputActions
{

/**
 * Minimum amount of deltas required to accurately detect axis changes.
 */
static const size_t AXIS_CHANGE_MIN_DELTA_COUNT = 10;

static const qreal CIRCLE_COASTING_FRICTION = 0.02;
static const std::chrono::milliseconds CIRCLE_COASTING_TIMER_INTERVAL{30L};
static const qreal PI_2 = M_PI * 2;

MotionTriggerHandler::MotionTriggerHandler()
{
    connect(this, &TriggerHandler::activatingTrigger, this, &MotionTriggerHandler::onActivatingTrigger);
    connect(this, &TriggerHandler::endingTriggers, this, &MotionTriggerHandler::onEndingTriggers);

    setSpeedThreshold(TriggerType::Pinch, 0.04, static_cast<TriggerDirection>(PinchDirection::In));
    setSpeedThreshold(TriggerType::Pinch, 0.08, static_cast<TriggerDirection>(PinchDirection::Out));
    setSpeedThreshold(TriggerType::Rotate, 5);
    setSpeedThreshold(TriggerType::Swipe, 20);

    connect(&m_circleCoastingTimer, &QTimer::timeout, this, &MotionTriggerHandler::onCircleCoastingTimerTick);
}

void MotionTriggerHandler::setSpeedThreshold(TriggerType type, qreal threshold, TriggerDirection directions)
{
    for (auto it = m_speedThresholds.begin(); it != m_speedThresholds.end();) {
        auto thresholds = *it;
        if (thresholds.type == type && thresholds.directions == directions) {
            it = m_speedThresholds.erase(it);
            continue;
        }
        it++;
    }
    m_speedThresholds.push_back({
        .type = type,
        .threshold = threshold,
        .directions = directions,
    });
}

bool MotionTriggerHandler::handleMotion(const InputDevice *device, const PointDelta &delta)
{
    if (!hasActiveTriggers(TriggerType::SinglePointMotion)) {
        return false;
    }

    qCDebug(INPUTACTIONS_HANDLER_MOTION).nospace() << "Event (type: Motion, delta: " << delta.unaccelerated() << ")";

    m_deltas.push_back(delta.unaccelerated());
    m_totalSwipeDelta += delta.unaccelerated();

    TriggerSpeed speed{};
    if (!determineSpeed(TriggerType::Swipe, delta.unacceleratedHypot(), speed)) {
        return true;
    }

    std::map<TriggerType, const TriggerUpdateEvent *> events;
    DirectionalMotionTriggerUpdateEvent circleEvent;
    DirectionalMotionTriggerUpdateEvent swipeEvent;
    MotionTriggerUpdateEvent strokeEvent;
    bool axisChanged{};

    // Block the event even if the result says not to do so.
    bool block{};

    if (hasActiveTriggers(TriggerType::Circle)) {
        m_circleCoastingTimer.start(CIRCLE_COASTING_TIMER_INTERVAL);

        const auto d = delta.unaccelerated();
        const auto angle = std::atan2(d.y(), d.x());

        auto angleDelta = angle - m_circlePreviousAngle;
        angleDelta = angleDelta - static_cast<int>(angleDelta / PI_2) * PI_2;
        if (angleDelta < 0) {
            angleDelta += PI_2;
        }
        if (angleDelta > M_PI) {
            angleDelta -= PI_2;
        }

        const auto absAngleDelta = std::abs(angleDelta);
        const auto distance = Math::hypot(d);

        // Clamp spikes
        if (absAngleDelta > 0.5) {
            angleDelta = 0.5 * angleDelta / absAngleDelta;
        }

        // Error estimation
        const auto angleDeltaError = std::pow(angleDelta - m_circleFilterDelta, 2);

        // Naive filter for error estimation on next event
        m_circleFilterDelta = (angleDelta + m_circleFilterDelta) / 2;

        // Reduce weight for small movements
        const auto minDistance = std::min(distance, m_circlePreviousDistance);
        const auto distanceFactor = std::log(1 + minDistance / 10);

        // Adaptive filter
        const auto weight = std::min(absAngleDelta * distanceFactor / (1 + angleDeltaError * 100), static_cast<qreal>(1));
        m_circleAdaptiveDelta = angleDelta * weight + m_circleAdaptiveDelta * (1 - weight);

        m_circlePreviousAngle = angle;
        m_circlePreviousDistance = distance;

        if (m_circleIsFirstEvent) {
            // First event will have a delta of 0, ignore
            m_circleIsFirstEvent = false;
            return hasActiveBlockingTriggers(TriggerType::Circle);
        }

        const auto cubedAngleDelta = std::pow(m_circleAdaptiveDelta, 3) * 10000; // Multiply by an arbitrary number to prevent decimal places in update
        // action intervals. The delta is not the actual angle anyway so it doesn't matter.
        m_circleTotalDelta += cubedAngleDelta;

        if (m_circleTotalDelta == 0) {
            // Cannot determine direction
            block = hasActiveBlockingTriggers(TriggerType::Circle);
        } else {
            circleEvent.setDelta(cubedAngleDelta);
            circleEvent.setDirection(static_cast<TriggerDirection>(m_circleTotalDelta > 0 ? RotateDirection::Clockwise : RotateDirection::Counterclockwise));
            circleEvent.setSpeed(speed);
            events[TriggerType::Circle] = &circleEvent;
        }

        // TODO: Cancel if motion is a straight line
    }

    if (hasActiveTriggers(TriggerType::Swipe)) {
        if (m_deltas.size() < 2) {
            // One delta may not be enough to determine the direction
            return true;
        }

        if (m_currentSwipeAxis == Axis::None) {
            m_currentSwipeAxis = std::abs(m_totalSwipeDelta.x()) >= std::abs(m_totalSwipeDelta.y()) ? Axis::Horizontal : Axis::Vertical;
        } else if (m_deltas.size() >= AXIS_CHANGE_MIN_DELTA_COUNT) { // Make sure we have enough data to detect axis change
            const std::vector<QPointF> lastDeltas(m_deltas.end() - AXIS_CHANGE_MIN_DELTA_COUNT, m_deltas.end());
            QPointF sum;
            for (const auto &delta : lastDeltas) {
                sum += {std::abs(delta.x()), std::abs(delta.y())};
            }

            if (std::min(sum.x(), sum.y()) / std::max(sum.x(), sum.y()) <= 0.2 // Must be a sharp turn
                && ((m_currentSwipeAxis == Axis::Horizontal && sum.y() > sum.x()) || (m_currentSwipeAxis == Axis::Vertical && sum.x() > sum.y()))) {
                m_currentSwipeAxis = m_currentSwipeAxis == Axis::Horizontal ? Axis::Vertical : Axis::Horizontal;
                axisChanged = true;
                qCDebug(INPUTACTIONS_HANDLER_MOTION, "Swipe axis changed");
            }
        }

        SwipeDirection direction{};
        switch (m_currentSwipeAxis) {
            case Axis::Vertical:
                direction = m_totalSwipeDelta.y() < 0 ? SwipeDirection::Up : SwipeDirection::Down;
                break;
            case Axis::Horizontal:
                direction = m_totalSwipeDelta.x() < 0 ? SwipeDirection::Left : SwipeDirection::Right;
                break;
            default:
                Q_UNREACHABLE();
        }

        swipeEvent.setDelta(m_currentSwipeAxis == Axis::Vertical ? Delta(delta.accelerated().y(), delta.unaccelerated().y())
                                                                 : Delta(delta.accelerated().x(), delta.unaccelerated().x()));
        swipeEvent.setDirection(static_cast<TriggerDirection>(direction));
        swipeEvent.setPointDelta({delta.accelerated() * m_swipeDeltaMultiplier, delta.unaccelerated() * m_swipeDeltaMultiplier});
        swipeEvent.setSpeed(speed);
        events[TriggerType::Swipe] = &swipeEvent;
    }

    if (hasActiveTriggers(TriggerType::Stroke)) {
        strokeEvent.setDelta(device->type() == InputDeviceType::Mouse ? delta.acceleratedHypot() : delta.unacceleratedHypot()); // backwards compatibility
        strokeEvent.setSpeed(speed);
        events[TriggerType::Stroke] = &strokeEvent;
    }

    const auto result = updateTriggers(events);
    if (axisChanged && !result.success) {
        activateTriggers(TriggerType::Swipe);
        return handleMotion(device, delta);
    }
    return result.block || block;
}

bool MotionTriggerHandler::determineSpeed(TriggerType type, qreal delta, TriggerSpeed &speed, TriggerDirection direction)
{
    if (!m_isDeterminingSpeed) {
        if (m_speed) {
            speed = *m_speed;
        }
        return true;
    }

    std::optional<TriggerSpeedThreshold> speedThreshold;
    for (const auto &threshold : m_speedThresholds) {
        if (threshold.type == type && threshold.directions & direction) {
            speedThreshold = threshold;
            break;
        }
    }
    if (!speedThreshold.has_value()) {
        qCWarning(INPUTACTIONS_HANDLER_MOTION, "No matching speed threshold found for trigger, assuming fast speed.");
        m_speed = speed = TriggerSpeed::Fast;
        return false;
    }

    if (m_sampledInputEvents++ != m_inputEventsToSample) {
        m_accumulatedAbsoluteSampledDelta += std::abs(delta);
        qCDebug(INPUTACTIONS_HANDLER_MOTION).noquote() << QString("Determining speed (event: %1/%2, delta: %3/%4)")
                                                              .arg(QString::number(m_sampledInputEvents),
                                                                   QString::number(m_inputEventsToSample),
                                                                   QString::number(m_accumulatedAbsoluteSampledDelta),
                                                                   QString::number(speedThreshold->threshold));
        return false;
    }

    m_isDeterminingSpeed = false;
    m_speed = speed = (m_accumulatedAbsoluteSampledDelta / m_inputEventsToSample) >= speedThreshold->threshold ? TriggerSpeed::Fast : TriggerSpeed::Slow;
    qCDebug(INPUTACTIONS_HANDLER_MOTION).noquote() << "Speed determined (speed: " << speed << ")";
    return true;
}

void MotionTriggerHandler::reset()
{
    TriggerHandler::reset();
    m_currentSwipeAxis = Axis::None;
    m_totalSwipeDelta = {};
    m_speed = {};
    m_isDeterminingSpeed = false;
    m_circleIsFirstEvent = true;
    m_deltas.clear();
    m_sampledInputEvents = m_accumulatedAbsoluteSampledDelta = m_circlePreviousAngle = m_circlePreviousDistance = m_circleFilterDelta = m_circleAdaptiveDelta
        = m_circleTotalDelta = 0;
    m_circleCoastingTimer.stop();
}

void MotionTriggerHandler::onCircleCoastingTimerTick()
{
    if (!hasActiveTriggers(TriggerType::Circle)) {
        m_circleCoastingTimer.stop();
    }

    if (m_circleAdaptiveDelta > CIRCLE_COASTING_FRICTION) {
        m_circleAdaptiveDelta -= CIRCLE_COASTING_FRICTION;
    } else if (m_circleAdaptiveDelta < -CIRCLE_COASTING_FRICTION) {
        m_circleAdaptiveDelta += CIRCLE_COASTING_FRICTION;
    } else {
        m_circleAdaptiveDelta = 0;
        m_circleCoastingTimer.stop();
    }
    m_circleFilterDelta = 0;
}

void MotionTriggerHandler::onActivatingTrigger(const Trigger *trigger)
{
    if (const auto motionTrigger = dynamic_cast<const MotionTrigger *>(trigger)) {
        if (!m_isDeterminingSpeed && motionTrigger->hasSpeed()) {
            qCDebug(INPUTACTIONS_HANDLER_MOTION).noquote() << QString("Trigger has speed (id: %1)").arg(trigger->id());
            m_isDeterminingSpeed = true;
        }
    }
}

void MotionTriggerHandler::onEndingTriggers(TriggerTypes types)
{
    if (m_deltas.empty() || !(types & TriggerType::Stroke)) {
        return;
    }

    const Stroke stroke(m_deltas);
    qCDebug(INPUTACTIONS_HANDLER_MOTION).noquote()
        << QString("Stroke constructed (points: %1, deltas: %2)").arg(QString::number(stroke.points().size()), QString::number(m_deltas.size()));

    Trigger *best = nullptr;
    double bestScore = 0;
    for (const auto &trigger : activeTriggers(TriggerType::Stroke)) {
        if (!trigger->canEnd()) {
            continue;
        }

        for (const auto &triggerStroke : dynamic_cast<StrokeTrigger *>(trigger)->strokes()) {
            const auto score = stroke.compare(triggerStroke);
            if (score > bestScore && score > Stroke::min_matching_score()) {
                best = trigger;
                bestScore = score;
            }
        }
    }
    qCDebug(INPUTACTIONS_HANDLER_MOTION).noquote() << QString("Stroke compared (bestScore: %2)").arg(QString::number(bestScore));

    if (best) {
        cancelTriggers(best);
        best->end();
    }
    cancelTriggers(TriggerType::Stroke); // TODO Double cancellation
}

}