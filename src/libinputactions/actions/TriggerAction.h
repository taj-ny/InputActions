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

    const qreal &value() const;
    /**
     * @param value Will be converted to an absolute value. 0 means execute exactly once per input event, direction
     * still applies. Default is 0.
     */
    void setValue(qreal value);

    /**
     * @param direction Default is Any.
     */
    void setDirection(const IntervalDirection &direction);

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
    /**
     * @param action If nullptr, will be constructed.
     */
    TriggerAction(std::shared_ptr<Action> action = {});
    virtual ~TriggerAction();

    /**
     * Called by the trigger.
     * @internal
     */
    void triggerStarted();
    /**
     * Called by the trigger.
     * @internal
     */
    void triggerUpdated(const Delta &delta, const PointDelta &deltaPointMultiplied);
    /**
     * Called by the trigger.
     * @internal
     */
    void triggerTick(qreal delta);
    /**
     * Called by the trigger.
     * @internal
     */
    void triggerEnded();
    /**
     * Called by the trigger.
     * @internal
     */
    void triggerCancelled();

    /**
     * Executes the action if it can be executed.
     * @see canExecute
     */
    void tryExecute();
    /**
     * @return Whether the condition and threshold are satisfied.
     */
    bool canExecute() const;

    const Action *action() const;

    /**
     * The point of the trigger's lifecycle at which the action should be executed.
     */
    On m_on = On::End;
    /**
     * How often and when an update action should repeat.
     */
    ActionInterval m_interval;
    /**
     * Use the accelerated delta for intervals, if available. This does not affect thresholds.
     */
    bool m_accelerated{};
    /**
     * Sets how far the trigger needs to progress in order for the action to be executed. Thresholds are always
     * positive.
     * @remark Begin actions can't have thresholds. Set the threshold on the trigger instead.
     */
    std::optional<Range<qreal>> m_threshold;

private:
    void update(const Delta &delta);

    /**
     * Resets member variables that hold information about the performed input action.
     */
    void reset();

    std::shared_ptr<Action> m_action;

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