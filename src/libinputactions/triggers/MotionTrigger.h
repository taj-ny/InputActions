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

#include "Trigger.h"

namespace libinputactions
{

class MotionTriggerUpdateEvent : public TriggerUpdateEvent
{
public:
    MotionTriggerUpdateEvent() = default;

    // Speed should be in a TriggerBeginEvent, but that's not a thing, and adding it would complicate everything.
    // Not worth it for a single property.
    TriggerSpeed m_speed = TriggerSpeed::Any;
    QPointF m_deltaMultiplied{};
};

/**
 * An input action that involves directionless motion.
 */
class MotionTrigger : public Trigger
{
public:
    MotionTrigger(TriggerType type = TriggerType::None);

    /**
     * @return Whether the speed matches.
     * @see Trigger::canUpdate
     * @internal
     */
    bool canUpdate(const TriggerUpdateEvent *event) const override;

    bool hasSpeed() const;
    /**
     * Ignored unless set.
     * @param speed Speed the gesture must be performed at.
     */
    void setSpeed(TriggerSpeed speed);

protected:
    void updateActions(const TriggerUpdateEvent *event) override;

private:
    TriggerSpeed m_speed = TriggerSpeed::Any;
};

}