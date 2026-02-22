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
#include <libinputactions/helpers/Math.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/input/devices/InputDeviceProperties.h>

namespace InputActions
{

/**
 * Angle tolerance for left, up, right and down directions. Remaining space is used for diagonals.
 */
static const qreal ANGLE_TOLERANCE = 20;
// clang-format off
static const std::unordered_map<SwipeTriggerDirection, std::tuple<qreal, qreal, bool>> ANGLES{
    // direction                             // min angle            // max angle           // bidirectional
    {SwipeTriggerDirection::Left,            {180 - ANGLE_TOLERANCE, 180 + ANGLE_TOLERANCE, false}},
    {SwipeTriggerDirection::Right,           {360 - ANGLE_TOLERANCE, ANGLE_TOLERANCE,       false}},
    {SwipeTriggerDirection::Up,              {90 - ANGLE_TOLERANCE,  90 + ANGLE_TOLERANCE,  false}},
    {SwipeTriggerDirection::Down,            {270 - ANGLE_TOLERANCE, 270 + ANGLE_TOLERANCE, false}},

    {SwipeTriggerDirection::LeftUp,          {90 + ANGLE_TOLERANCE,  180 - ANGLE_TOLERANCE, false}},
    {SwipeTriggerDirection::LeftDown,        {180 + ANGLE_TOLERANCE, 270 - ANGLE_TOLERANCE, false}},
    {SwipeTriggerDirection::RightUp,         {ANGLE_TOLERANCE,       90 - ANGLE_TOLERANCE,  false}},
    {SwipeTriggerDirection::RightDown,       {270 + ANGLE_TOLERANCE, 360 - ANGLE_TOLERANCE, false}},

    {SwipeTriggerDirection::LeftRight,       {360 - ANGLE_TOLERANCE, ANGLE_TOLERANCE,       true}},
    {SwipeTriggerDirection::UpDown,          {270 - ANGLE_TOLERANCE, 270 + ANGLE_TOLERANCE, true}},

    {SwipeTriggerDirection::LeftUpRightDown, {270 + ANGLE_TOLERANCE, 360 - ANGLE_TOLERANCE, true}},
    {SwipeTriggerDirection::LeftDownRightUp, {ANGLE_TOLERANCE,       90 - ANGLE_TOLERANCE,  true}},

    {SwipeTriggerDirection::Any,             {0,                     360,                   false}},
};
// clang-format on

SwipeTrigger::SwipeTrigger(qreal minAngle, qreal maxAngle)
    : MotionTrigger(TriggerType::Swipe)
    , m_minAngle(minAngle)
    , m_maxAngle(maxAngle)
{
}

SwipeTrigger::SwipeTrigger(SwipeTriggerDirection direction)
    : MotionTrigger(TriggerType::Swipe)
{
    const auto &angles = ANGLES.at(direction);
    m_minAngle = std::get<0>(angles);
    m_maxAngle = std::get<1>(angles);
    m_bidirectional = std::get<2>(angles);
}

bool SwipeTrigger::canUpdate(const TriggerUpdateEvent &event) const
{
    if (!MotionTrigger::canUpdate(event)) {
        return false;
    }

    const auto &castedEvent = dynamic_cast<const SwipeTriggerUpdateEvent &>(event);
    const auto angle = castedEvent.averageAngle(); // Use the average so that the trigger is not cancelled on jitter
    return matchesAngleRange(angle) || (m_bidirectional && matchesOppositeAngleRange(angle));
}

void SwipeTrigger::updateActions(const TriggerUpdateEvent &event)
{
    auto newEvent = dynamic_cast<const SwipeTriggerUpdateEvent &>(event);
    const auto angle = newEvent.angle();

    // Ensure delta is always positive for normal angle range, and negative for opposite. Normal range takes priority over the opposite one in case of
    // overlapping.
    if (!matchesAngleRange(angle) && matchesOppositeAngleRange(angle)) {
        auto delta = event.delta();
        delta = {delta.accelerated() * -1, delta.unaccelerated() * -1};
        newEvent.setDelta(delta);
    }

    MotionTrigger::updateActions(newEvent);
}

bool SwipeTrigger::matchesAngleRange(qreal angle) const
{
    if (m_minAngle <= m_maxAngle) {
        return angle >= m_minAngle && angle <= m_maxAngle;
    }
    return angle >= m_minAngle || angle <= m_maxAngle;
}

bool SwipeTrigger::matchesOppositeAngleRange(qreal angle) const
{
    auto min = m_minAngle - 180;
    if (min < 0) {
        min += 360;
    }

    auto max = m_maxAngle - 180;
    if (max < 0) {
        max += 360;
    }

    if (min <= max) {
        return angle >= min && angle <= max;
    }
    return angle >= min || angle <= max;
}

}