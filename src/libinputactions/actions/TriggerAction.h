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

#include <QLoggingCategory>
#include <QPointF>
#include <QString>
#include <libinputactions/Range.h>
#include <libinputactions/conditions/Condition.h>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS_ACTION)

namespace InputActions
{

class Action;
class Delta;
class PointDelta;

/**
 * The point of the trigger's lifecycle at which the action should be executed.
 */
enum class On
{
    Begin,
    Cancel,
    End,
    EndCancel,
    Tick,
    Update
};

enum class IntervalDirection
{
    /**
     * The update delta can be either positive or negative.
     */
    Any,
    /**
     * The update delta must be positive.
     */
    Positive,
    /**
     * The update delta must be negative.
     */
    Negative
};

/**
 * Defines how often and when should an action repeat.
 */
class ActionInterval
{
public:
    /**
     * @return Whether the specified delta matches the interval's direction.
     */
    bool matches(qreal delta) const;

    qreal value() const { return m_value; }
    /**
     * @param value Will be converted to an absolute value. 0 means execute exactly once per input event, direction still applies. Default is 0.
     */
    void setValue(qreal value) { m_value = value; }

    /**
     * Default is Any.
     */
    void setDirection(IntervalDirection value) { m_direction = value; }

private:
    qreal m_value{};
    IntervalDirection m_direction = IntervalDirection::Any;
};

/**
 * Executed at a specific point of the trigger's lifecycle.
 */
class TriggerAction
{
public:
    TriggerAction();
    TriggerAction(std::unique_ptr<Action> action);
    virtual ~TriggerAction();

    /**
     * Called by the trigger.
     */
    void triggerStarted();
    /**
     * Called by the trigger.
     */
    void triggerUpdated(const Delta &delta, const PointDelta &deltaPointMultiplied);
    /**
     * Called by the trigger.
     */
    void triggerTick(qreal delta);
    /**
     * Called by the trigger.
     */
    void triggerEnded();
    /**
     * Called by the trigger.
     */
    void triggerCancelled();

    /**
     * Executes the action if it can be executed.
     * @param executions Must be 1 if the action is not mergeable.
     * @see canExecute
     */
    void tryExecute(uint32_t executions = 1);
    /**
     * @return Whether the condition and threshold are satisfied.
     */
    bool canExecute() const;

    const Action *action() const { return m_action.get(); }

    /**
     * The point of the trigger's lifecycle at which the action should be executed.
     */
    On on() const { return m_on; }
    void setOn(On value) { m_on = value; }

    /**
     * How often and when an update action should repeat.
     */
    const ActionInterval &interval() const { return m_interval; }
    void setInterval(ActionInterval value) { m_interval = std::move(value); }

    /**
     * Use the accelerated delta for intervals, if available. This does not affect thresholds.
     */
    bool accelerated() const { return m_accelerated; }
    void setAccelerated(bool value) { m_accelerated = value; }

    /**
     * How far the trigger needs to progress in order for the action to be executed. Thresholds are always positive.
     * @remark Begin actions can't have thresholds. Set the threshold on the trigger instead.
     */
    const std::optional<Range<qreal>> &threshold() const { return m_threshold; }
    void setThreshold(Range<qreal> value) { m_threshold = std::move(value); }

    /**
     * Whether this action can activate conflict resolution.
     */
    bool conflicting() const { return m_conflicting; }
    void setConflicting(bool value) { m_conflicting = value; }

private:
    void update(const Delta &delta);

    /**
     * Resets member variables that hold information about the performed input action.
     */
    void reset();

    bool m_accelerated{};
    std::unique_ptr<Action> m_action;
    ActionInterval m_interval;
    On m_on = On::End;
    std::optional<Range<qreal>> m_threshold;
    bool m_conflicting = true;

    /**
     * The sum of deltas from update events. Reset when the direction changes.
     */
    qreal m_accumulatedDelta{};
    /**
     * The sum of absolute deltas from updates event, used for thresholds.
     */
    qreal m_absoluteAccumulatedDelta{};

    friend class TestTrigger;
};

}