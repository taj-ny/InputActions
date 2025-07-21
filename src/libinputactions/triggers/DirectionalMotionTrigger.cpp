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

#include "DirectionalMotionTrigger.h"

namespace libinputactions
{

DirectionalMotionTrigger::DirectionalMotionTrigger(TriggerType type, TriggerDirection direction)
    : MotionTrigger(type)
    , m_direction(direction)
{
}

bool DirectionalMotionTrigger::canUpdate(const TriggerUpdateEvent *event) const
{
    if (!MotionTrigger::canUpdate(event)) {
        return false;
    }

    const auto *castedEvent = dynamic_cast<const DirectionalMotionTriggerUpdateEvent *>(event);
    return m_direction & castedEvent->m_direction;
}

void DirectionalMotionTrigger::updateActions(const TriggerUpdateEvent *event)
{
    const auto *castedEvent = dynamic_cast<const DirectionalMotionTriggerUpdateEvent *>(event);

    // Ensure delta is always positive for single-directional gestures, it makes intervals easier to use.
    static std::vector<TriggerDirection> negativeDirections = {
        static_cast<TriggerDirection>(PinchDirection::In),
        static_cast<TriggerDirection>(RotateDirection::Counterclockwise),
        static_cast<TriggerDirection>(SwipeDirection::Left),
        static_cast<TriggerDirection>(SwipeDirection::Up),
    };
    auto delta = castedEvent->m_delta;
    if ((m_direction & (m_direction - 1)) == 0 && std::find(negativeDirections.begin(), negativeDirections.end(), m_direction) != negativeDirections.end()) {
        delta *= -1;
    }

    for (auto &action : actions()) {
        action->triggerUpdated(delta, castedEvent->m_deltaMultiplied);
    }
}

}