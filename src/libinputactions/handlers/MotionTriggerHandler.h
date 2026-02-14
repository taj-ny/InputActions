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

#pragma once

#include "InputTriggerHandler.h"
#include <libinputactions/triggers/DirectionalMotionTrigger.h>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS_HANDLER_MOTION)

namespace InputActions
{

class PointDelta;

enum class Axis
{
    Horizontal,
    Vertical,
    None
};

struct TriggerSpeedThreshold
{
    TriggerType type;
    qreal threshold;
    uint32_t directions;
};

/**
 * Handles motion triggers: stroke, swipe.
 */
class MotionTriggerHandler : public InputTriggerHandler
{
public:
    /**
     * Duplicate thresholds (same type and direction) will be replaced.
     */
    void setSpeedThreshold(TriggerType type, qreal threshold, TriggerDirection directions = UINT32_MAX);

    /**
     * Global move_by_delta delta multiplier. Deprecated, use InputAction::Item::mouseMoveRelativeByDelta instead.
     */
    void setSwipeDeltaMultiplier(qreal value) { m_swipeDeltaMultiplier = value; }

    /**
     * How many input events to sample in order to determine the speed.
     */
    void setInputEventsToSample(uint8_t value) { m_inputEventsToSample = value; }

protected:
    MotionTriggerHandler();

    /**
     * Does nothing if there are no active pinch or rotate triggers.
     * @return Whether there are any active pinch or rotate triggers.
     */
    TEST_VIRTUAL bool handleMotion(const InputDevice *device, const PointDelta &delta);

    /**
     * If false is returned, speed is being determined and methods processing triggers must also return true
     * immediately and not update triggers.
     * @param delta The delta of the individual input event.
     * @param speed Set to the speed once it is determined.
     * @return Whether speed is necessary and has been determined.
     * @see setSpeedThreshold
     */
    bool determineSpeed(TriggerType type, qreal delta, TriggerSpeed &speed, TriggerDirection direction = UINT32_MAX);

    void reset() override;

private slots:
    void onCircleCoastingTimerTick();

    void onActivatingTrigger(const Trigger *trigger);
    void onEndingTriggers(TriggerTypes types);

private:
    Axis m_currentSwipeAxis = Axis::None;
    QPointF m_totalSwipeDelta;

    bool m_isDeterminingSpeed = false;
    uint8_t m_sampledInputEvents = 0;
    qreal m_accumulatedAbsoluteSampledDelta = 0;
    std::optional<TriggerSpeed> m_speed;
    std::vector<TriggerSpeedThreshold> m_speedThresholds;

    qreal m_circleTotalDelta{};
    qreal m_circlePreviousAngle{};
    qreal m_circlePreviousDistance{};
    qreal m_circleFilterDelta{};
    qreal m_circleAdaptiveDelta{};
    bool m_circleIsFirstEvent = true;
    QTimer m_circleCoastingTimer;

    std::vector<QPointF> m_deltas;

    qreal m_swipeDeltaMultiplier = 1.0;
    uint8_t m_inputEventsToSample = 3;

    friend class MockTouchpadTriggerHandler;
};

}