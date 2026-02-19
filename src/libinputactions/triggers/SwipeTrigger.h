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

#include "DirectionalMotionTrigger.h"

namespace InputActions
{

class SwipeTriggerUpdateEvent : public MotionTriggerUpdateEvent
{
public:
    SwipeTriggerUpdateEvent() = default;

    qreal angle() const { return m_angle; }
    void setAngle(qreal value) { m_angle = value; }

    qreal averageAngle() const { return m_averageAngle; }
    void setAverageAngle(qreal value) { m_averageAngle = value; }

private:
    qreal m_angle{};
    qreal m_averageAngle{};
};

class SwipeTrigger : public MotionTrigger
{
public:
    SwipeTrigger(qreal angleMin, qreal angleMax);
    SwipeTrigger(SwipeDirection swipeDirection);

    /**
     * Whether motion in the opposite angle range is valid as well. Such motion will have a negative delta
     */
    void setBidirectional(bool value) { m_bidirectional = value; }

    bool canUpdate(const TriggerUpdateEvent &event) const override;

protected:
    void updateActions(const TriggerUpdateEvent &event) override;

private:
    bool matchesAngleRange(qreal angle) const;
    bool matchesOppositeAngleRange(qreal angle) const;

    qreal m_angleMin;
    qreal m_angleMax;
    bool m_bidirectional{};
};

}