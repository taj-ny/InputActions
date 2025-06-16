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

#pragma once

#include <libinputactions/triggers/DirectionalMotionTrigger.h>
#include "TriggerHandler.h"

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS_HANDLER_MOTION)

namespace libinputactions
{

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
class MotionTriggerHandler : public TriggerHandler
{
public:
    /**
     * Duplicate thresholds (same type and direction) will be replaced.
     */
    void setSpeedThreshold(const TriggerType &type, const qreal &threshold, const TriggerDirection &directions = UINT32_MAX);
    void setSpeedInputEventsToSample(const uint8_t &events);

    /**
     * Used in input actions, as KWin doesn't provide accelerated deltas for gestures. Temporary workaround.
     */
    void setSwipeDeltaMultiplier(const qreal &multiplier);

protected:
    MotionTriggerHandler();

    /**
     * Does nothing if there are no active pinch or rotate triggers.
     * @return Whether there are any active pinch or rotate triggers.
     */
    bool handleMotion(const QPointF &delta);

    /**
     * If false is returned, speed is being determined and methods processing triggers must also return true
     * immediately and not update triggers.
     * @param delta The delta of the individual input event.
     * @param speed Set to the speed once it is determined.
     * @return Whether speed is necessary and has been determined.
     * @see setSpeedThreshold
     */
    bool determineSpeed(const TriggerType &type, const qreal &delta, TriggerSpeed &speed, const TriggerDirection &direction = UINT32_MAX);

    virtual void triggerActivating(const Trigger *trigger) override;
    void reset() override;

private:
    void strokeTriggerEndHandler();

    Axis m_currentSwipeAxis = Axis::None;
    QPointF m_currentSwipeDelta;
    qreal m_swipeDeltaMultiplier = 1.0;

    bool m_isDeterminingSpeed = false;
    uint8_t m_inputEventsToSample = 3;
    uint8_t m_sampledInputEvents = 0;
    qreal m_accumulatedAbsoluteSampledDelta = 0;
    std::optional<TriggerSpeed> m_speed;
    std::vector<TriggerSpeedThreshold> m_speedThresholds;

    std::vector<QPointF> m_stroke;
};

}