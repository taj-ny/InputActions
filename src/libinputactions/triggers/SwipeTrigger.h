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

#include "MotionTrigger.h"

namespace InputActions
{

class SwipeTriggerUpdateEvent : public MotionTriggerUpdateEvent
{
public:
    SwipeTriggerUpdateEvent() = default;

    /**
     * Angle for this event.
     */
    qreal angle() const { return m_angle; }
    void setAngle(qreal value) { m_angle = value; }

    /**
     * Average angle derived from all events in the window based on the motion threshold.
     */
    qreal averageAngle() const { return m_averageAngle; }
    void setAverageAngle(qreal value) { m_averageAngle = value; }

private:
    qreal m_angle{};
    qreal m_averageAngle{};
};

/**
 * For bidirectional values, the first direction named in the enum will always have a negative delta, and the second direction will have a positive one.
 */
enum class SwipeTriggerDirection
{
    Left,
    Right,
    Up,
    Down,

    // Diagonal
    LeftUp,
    LeftDown,
    RightUp,
    RightDown,

    // Bidirectional
    LeftRight,
    UpDown,

    // Diagonal bidirectional
    LeftUpRightDown,
    LeftDownRightUp,

    Any
};

/**
 * An input action that involves motion at a particular angle.
 *
 * The direction is determined when the motion threshold is reached. In case of issues regarding direction detection, update the appropriate motion threshold
 * in DeviceProperties. Touchpads have 3 different thresholds, depending on the finger count.
 *
 * If swipe triggers are active and the motion angle changes, but none of the active triggers acccept it, they are cancelled and all swipe triggers are
 * activated again, allowing for chaining multiple triggers together.
 */
class SwipeTrigger : public MotionTrigger
{
public:
    /**
     * If minAngle < maxAngle, the range includes all values where x >= minAngle && x <= maxAngle.
     * If minAngle > maxAngle, the range includes all values where x >= minAngle || x <= maxAngle.
     */
    SwipeTrigger(qreal minAngle, qreal maxAngle);
    SwipeTrigger(SwipeTriggerDirection swipeDirection);

    qreal minAngle() const { return m_minAngle; }
    qreal maxAngle() const { return m_maxAngle; }

    /**
     * Whether motion in the opposite angle range is valid as well. Such motion will have a negative delta. In case of overlapping ranges, the normal one has
     * the higher priority.
     */
    void setBidirectional(bool value) { m_bidirectional = value; }

    bool canUpdate(const TriggerUpdateEvent &event) const override;

protected:
    void updateActions(const TriggerUpdateEvent &event) override;

private:
    bool matchesAngleRange(qreal angle) const;
    bool matchesOppositeAngleRange(qreal angle) const;

    qreal m_minAngle;
    qreal m_maxAngle;
    bool m_bidirectional{};

    friend class TestSwipeTrigger;
};

}