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

#include <libinputactions/conditions/Condition.h>
#include <libinputactions/Range.h>

#include <memory>

#include <QLoggingCategory>
#include <QPointF>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS_ACTION)

namespace libinputactions
{

/**
 * The point of the trigger's lifecycle at which the action should be executed.
 */
enum class On {
    Begin,
    Cancel,
    End,
    EndCancel,
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
    bool matches(const qreal &delta) const;

    const qreal &value() const;
    /**
     * @param value Will be converted to an absolute value. 0 means execute exactly once per input event, direction
     * still applies. Default is 0.
     */
    void setValue(const qreal &value);

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
    virtual ~TriggerAction() = default;

    /**
     * Called by the trigger.
     * @internal
     */
    TEST_VIRTUAL void triggerStarted();
    /**
     * Called by the trigger.
     * @internal
     */
    TEST_VIRTUAL void triggerUpdated(const qreal &delta, const QPointF &deltaPointMultiplied);
    /**
     * Called by the trigger.
     * @internal
     */
    TEST_VIRTUAL void triggerEnded();
    /**
     * Called by the trigger.
     * @internal
     */
    TEST_VIRTUAL void triggerCancelled();

    /**
     * Executes the action if it can be executed.
     * @see canExecute
     */
    TEST_VIRTUAL void tryExecute();
    /**
     * @return Whether the action had been executed during the trigger.
     */
    TEST_VIRTUAL const bool &executed() const;
    /**
     * @return Whether the condition and threshold are satisfied.
     */
    TEST_VIRTUAL bool canExecute() const;

    /**
     * @param condition Must be satisfied in order for the action to be executed. To add multiple conditions, use a
     * condition group.
     */
    void setCondition(const std::shared_ptr<const Condition> &condition);

    const QString &name() const;
    /**
     * @param name Used in debug logs.
     */
    void setName(const QString &name);

    const On &on() const;
    /**
     * @param on The point of the trigger's lifecycle at which the action should be executed.
     */
    void setOn(const On &on);

    /**
     * @param interval How often and when an update action should repeat.
     */
    void setRepeatInterval(const ActionInterval &interval);

    /**
     * Sets how far the trigger needs to progress in order for the action to be executed. Thresholds are always
     * positive.
     * @remark Begin actions can't have thresholds. Set the threshold on the trigger instead.
     */
    void setThreshold(const Range<qreal> &threshold);

protected:
    TriggerAction() = default;

    /**
     * Executes the action without checking conditions.
     */
    virtual void execute() { };

    // This is just a quick way to get directionless swipe gestures working
    QPointF m_currentDeltaPointMultiplied;

private:
    /**
     * Resets member variables that hold information about the performed input action.
     */
    void reset();

    QString m_name = "Unnamed action";
    On m_on = On::End;
    ActionInterval m_interval;
    std::optional<Range<qreal>> m_threshold;
    std::optional<std::shared_ptr<const Condition>> m_condition;
    bool m_executed{};

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