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

#include "MotionTrigger.h"

namespace InputActions
{

MotionTrigger::MotionTrigger(TriggerType type)
    : Trigger(type)
{
}

bool MotionTrigger::canUpdate(const TriggerUpdateEvent &event) const
{
    const auto &castedEvent = dynamic_cast<const MotionTriggerUpdateEvent &>(event);
    return m_speed == TriggerSpeed::Any || m_speed == castedEvent.m_speed;
}

bool MotionTrigger::hasSpeed() const
{
    return m_speed != TriggerSpeed::Any;
}

void MotionTrigger::updateActions(const TriggerUpdateEvent &event)
{
    const auto &castedEvent = dynamic_cast<const MotionTriggerUpdateEvent &>(event);
    for (auto &action : actions()) {
        action->triggerUpdated(event.m_delta, castedEvent.m_deltaMultiplied);
    }
}

}