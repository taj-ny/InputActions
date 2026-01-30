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

#include "Trigger.h"

namespace InputActions
{

class MotionTriggerUpdateEvent : public TriggerUpdateEvent
{
public:
    MotionTriggerUpdateEvent() = default;

    TriggerSpeed speed() const { return m_speed; }
    void setSpeed(TriggerSpeed value) { m_speed = value; }

    PointDelta deltaMultiplied() const { return m_deltaMultiplied; }
    void setDeltaMultiplied(PointDelta value) { m_deltaMultiplied = std::move(value); }

private:
    // Speed should be in a TriggerBeginEvent, but that's not a thing, and adding it would complicate everything.
    // Not worth it for a single property.
    TriggerSpeed m_speed = TriggerSpeed::Any;
    PointDelta m_deltaMultiplied{};
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
    bool canUpdate(const TriggerUpdateEvent &event) const override;

    bool hasSpeed() const;
    TriggerSpeed speed() const { return m_speed; }
    void setSpeed(TriggerSpeed value) { m_speed = value; }

    /**
     * Lock the pointer while this trigger is active. Only applies to mouse triggers.
     */
    bool lockPointer() const { return m_lockPointer; }
    void setLockPointer(bool value) { m_lockPointer = value; }

protected:
    void updateActions(const TriggerUpdateEvent &event) override;

private:
    bool m_lockPointer{};
    TriggerSpeed m_speed = TriggerSpeed::Any;
};

}