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

#include "MotionTriggerHandler.h"
#include <libinputactions/triggers/StrokeTrigger.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_HANDLER_MOTION, "inputactions.handler.motion", QtWarningMsg)

namespace libinputactions
{

MotionTriggerHandler::MotionTriggerHandler()
{
    connect(this, &TriggerHandler::activatingTrigger, this, &MotionTriggerHandler::onActivatingTrigger);
    connect(this, &TriggerHandler::endingTriggers, this, &MotionTriggerHandler::onEndingTriggers);

    setSpeedThreshold(TriggerType::Pinch, 0.04, static_cast<TriggerDirection>(PinchDirection::In));
    setSpeedThreshold(TriggerType::Pinch, 0.08, static_cast<TriggerDirection>(PinchDirection::Out));
    setSpeedThreshold(TriggerType::Rotate, 5);
    setSpeedThreshold(TriggerType::Swipe, 20);
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

bool MotionTriggerHandler::handleMotion(const QPointF &delta)
{
    if (!hasActiveTriggers(TriggerType::StrokeSwipe)) {
        return false;
    }

    qCDebug(INPUTACTIONS_HANDLER_MOTION).nospace() << "Event (type: Motion, delta: " << delta << ")";

    const auto hasStroke = hasActiveTriggers(TriggerType::Stroke);
    if (hasStroke) {
        m_stroke.push_back(delta);
    }
    m_currentSwipeDelta += delta;

    const auto deltaHypot = std::hypot(delta.x(), delta.y());
    TriggerSpeed speed{};
    if (!determineSpeed(TriggerType::Swipe, deltaHypot, speed)) {
        qCDebug(INPUTACTIONS_HANDLER_MOTION, "Event processed (type: Motion, status: DeterminingSpeed)");
        return true;
    }

    std::map<TriggerType, const TriggerUpdateEvent *> events;
    DirectionalMotionTriggerUpdateEvent swipeEvent;
    MotionTriggerUpdateEvent strokeEvent;

    if (hasActiveTriggers(TriggerType::Swipe)) {
        SwipeDirection direction; // Overall direction
        Axis swipeAxis;

        // Pick an axis for gestures so horizontal ones don't change to vertical ones without lifting fingers
        if (m_currentSwipeAxis == Axis::None) {
            if (std::abs(m_currentSwipeDelta.x()) >= std::abs(m_currentSwipeDelta.y()))
                swipeAxis = Axis::Horizontal;
            else
                swipeAxis = Axis::Vertical;

            if (std::abs(m_currentSwipeDelta.x()) >= 5 || std::abs(m_currentSwipeDelta.y()) >= 5) {
                // only lock in a direction if the delta is big enough
                // to prevent accidentally choosing the wrong direction
                m_currentSwipeAxis = swipeAxis;
            }
        } else
            swipeAxis = m_currentSwipeAxis;

        // Find the current swipe direction
        switch (swipeAxis) {
            case Axis::Vertical:
                direction = m_currentSwipeDelta.y() < 0 ? SwipeDirection::Up : SwipeDirection::Down;
                break;
            case Axis::Horizontal:
                direction = m_currentSwipeDelta.x() < 0 ? SwipeDirection::Left : SwipeDirection::Right;
                break;
            default:
                Q_UNREACHABLE();
        }

        swipeEvent.m_delta = swipeAxis == Axis::Vertical ? delta.y() : delta.x();
        swipeEvent.m_direction = static_cast<TriggerDirection>(direction);
        swipeEvent.m_deltaMultiplied = delta * m_swipeDeltaMultiplier;
        swipeEvent.m_speed = speed;
        events[TriggerType::Swipe] = &swipeEvent;
    }

    if (hasStroke) {
        strokeEvent.m_delta = deltaHypot;
        strokeEvent.m_speed = speed;
        events[TriggerType::Stroke] = &strokeEvent;
    }

    const auto hasTriggers = updateTriggers(events);
    qCDebug(INPUTACTIONS_HANDLER_MOTION).nospace() << "Event processed (type: Motion, hasTriggers: " << hasTriggers << ")";
    return hasTriggers;
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
    m_currentSwipeDelta = {};
    m_speed = {};
    m_isDeterminingSpeed = false;
    m_sampledInputEvents = 0;
    m_accumulatedAbsoluteSampledDelta = 0;
    m_stroke.clear();
}

void MotionTriggerHandler::onActivatingTrigger(const Trigger *trigger)
{
    if (const auto motionTrigger = dynamic_cast<const MotionTrigger *>(trigger)) {
        if (!m_isDeterminingSpeed && motionTrigger->hasSpeed()) {
            qCDebug(INPUTACTIONS_HANDLER_MOTION).noquote() << QString("Trigger has speed (id: %1)").arg(trigger->m_id);
            m_isDeterminingSpeed = true;
        }
    }
}

void MotionTriggerHandler::onEndingTriggers(TriggerTypes types)
{
    if (m_stroke.empty() || !(types & TriggerType::Stroke)) {
        return;
    }

    const Stroke stroke(m_stroke);
    qCDebug(INPUTACTIONS_HANDLER_MOTION).noquote()
        << QString("Stroke constructed (points: %1, deltas: %2)").arg(QString::number(stroke.points().size()), QString::number(m_stroke.size()));

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