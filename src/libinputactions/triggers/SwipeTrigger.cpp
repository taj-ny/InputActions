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

#include "SwipeTrigger.h"

namespace InputActions
{

SwipeTrigger::SwipeTrigger(qreal angleMin, qreal angleMax)
    : MotionTrigger(TriggerType::Swipe)
    , m_angleMin(angleMin)
    , m_angleMax(angleMax)
{
}

SwipeTrigger::SwipeTrigger(SwipeDirection direction)
    : MotionTrigger(TriggerType::Swipe)
{
    switch (direction) {
        case SwipeDirection::Left: // 180
            m_angleMin = 170;
            m_angleMax = 190;
            break;
        case SwipeDirection::Right: // 0
            m_angleMin = 350;
            m_angleMax = 10;
            break;
        case SwipeDirection::LeftRight:
            m_angleMin = 350;
            m_angleMax = 10;
            m_bidirectional = true;
            break;
        case SwipeDirection::Up: // 90
            m_angleMin = 80;
            m_angleMax = 100;
            break;
        case SwipeDirection::Down: // 270
            m_angleMin = 260;
            m_angleMax = 280;
            break;
        case SwipeDirection::UpDown:
            m_angleMin = 260;
            m_angleMax = 280;
            m_bidirectional = true;
            break;
        case SwipeDirection::Any:
            m_angleMin = 0;
            m_angleMax = 360;
            break;
    }
}

bool SwipeTrigger::canUpdate(const TriggerUpdateEvent &event) const
{
    if (!MotionTrigger::canUpdate(event)) {
        return false;
    }

    const auto &castedEvent = dynamic_cast<const SwipeTriggerUpdateEvent &>(event);
    const auto averageAngle = castedEvent.averageAngle();
    return matchesAngleRange(averageAngle) || (m_bidirectional && matchesOppositeAngleRange(averageAngle));
}

void SwipeTrigger::updateActions(const TriggerUpdateEvent &event)
{
    const auto &castedEvent = dynamic_cast<const SwipeTriggerUpdateEvent &>(event);
    const auto angle = castedEvent.angle();

    // Ensure delta is always positive for normal angle range, and negative for opposite.
    auto delta = event.delta();
    if (matchesOppositeAngleRange(angle)) {
        delta = {delta.accelerated() * -1, delta.unaccelerated() * -1};
    }

    for (const auto &action : actions()) {
        action->triggerUpdated(delta, castedEvent.deltaMultiplied());
    }
}

bool SwipeTrigger::matchesAngleRange(qreal angle) const
{
    if (m_angleMin < m_angleMax) {
        return angle >= m_angleMin && angle <= m_angleMax;
    }
    return angle >= m_angleMin || angle <= m_angleMax;
}

bool SwipeTrigger::matchesOppositeAngleRange(qreal angle) const
{
    auto min = m_angleMin - 180;
    if (min < 0) {
        min += 360;
    }

    auto max = m_angleMax - 180;
    if (max < 0) {
        max += 360;
    }

    if (min < max) {
        return angle >= min && angle <= max;
    }
    return angle >= min || angle <= max;
}

}