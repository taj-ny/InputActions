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

#include <libinputactions/actions/action.h>
#include <libinputactions/conditions/condition.h>
#include <libinputactions/globals.h>

#include <QLoggingCategory>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(LIBINPUTACTIONS_TRIGGER)

namespace libinputactions
{

/**
 * Unset optional fields are not checked by triggers.
 */
class TriggerActivationEvent
{
public:
    std::optional<uint8_t> fingers;
    std::optional<Qt::KeyboardModifiers> keyboardModifiers;
    std::optional<Qt::MouseButtons> mouseButtons;
    std::optional<QPointF> position;
};
class TriggerUpdateEvent
{
public:
    TriggerUpdateEvent() = default;
    virtual ~TriggerUpdateEvent() = default;

    const qreal &delta() const;
    void setDelta(const qreal &delta);

private:
    qreal m_delta = 0;
};

/**
 * An input action that does not involve motion.
 *
 * Triggers are managed by a trigger handler.
 */
class Trigger
{
public:
    Trigger() = default;
    virtual ~Trigger() = default;

    void addAction(std::unique_ptr<TriggerAction> action);
    /**
     * @param condition Must be satisfied in order for the trigger to be activated.
     */
    void setActivationCondition(const std::shared_ptr<const Condition> &condition);
    /**
     * @param condition Must be satisfied in order for the trigger to end..
     */
    void setEndCondition(const std::shared_ptr<const Condition> &condition);

    /**
     * @return Whether conditions, fingers, keyboard modifiers, mouse buttons and begin positions are satisfied.
     * @internal
     */
    TEST_VIRTUAL bool canActivate(const TriggerActivationEvent *event) const;

    /**
     * Called by the trigger handler before updating a trigger. If true is returned, that trigger will be cancelled.
     * @internal
     */
    virtual bool canUpdate(const TriggerUpdateEvent *event) const;
    /**
     * @internal
     */
    TEST_VIRTUAL void update(const TriggerUpdateEvent *event);

    /**
     * Called by the trigger handler before ending a trigger. If true is returned, that trigger will be cancelled
     * instead of ended.
     * @return Whether the trigger can be ended.
     * @internal
     */
    bool canEnd() const;
    /**
     * Resets the trigger and notifies all actions that it has ended.
     * @internal
     */
    void end();

    /**
     * Resets the trigger and notifies all actions that it has been cancelled.
     * @internal
     */
    void cancel();

    /**
     * The trigger handler calls this method before ending a trigger. If true is returned, that trigger is ended and
     * all others are cancelled.
     * @return Whether the trigger has an action that executes on trigger and can be executed.
     * @internal
     */
    bool overridesOtherTriggersOnEnd();
    /**
     * The trigger handler calls this method after updating a trigger. If true is returned for one, all other triggers
     * are cancelled.
     * @return Whether the trigger has any action that has been executed or is an update action and can be executed.
     * @internal
     */
    bool overridesOtherTriggersOnUpdate();

    /**
     * Ignored unless set.
     *
     * @param threshold How far the gesture must progress in order to begin.
     */
    void setThreshold(const Range<qreal> &threshold);

    /**
     * @param value Whether keyboard modifiers should be cleared when this trigger starts. By default true if the
     * trigger has an input action, otherwise false.
     */
    void setClearModifiers(const bool &value);

    const std::optional<Qt::MouseButtons> &mouseButtons() const;
    /**
     * Only applies to mouse triggers.
     *
     * @param buttons Mouse buttons that must be pressed before and during the trigger.
     */
    void setMouseButtons(const std::optional<Qt::MouseButtons> &buttons);

    const QString &name() const;
    /**
     * @param name Shown in debug logs.
     */
    void setName(const QString &name);

    const TriggerType &type() const;
    /**
     * Required.
     */
    void setType(const TriggerType &type);

protected:
    const std::vector<TriggerAction *> actions();

    virtual void updateActions(const TriggerUpdateEvent *event);

private:
    void reset();

    QString m_name = "Unnamed gesture";
    TriggerType m_type{0};
    std::vector<std::unique_ptr<TriggerAction>> m_actions;
    bool m_started = false;
    std::optional<bool> m_clearModifiers;

    std::optional<std::shared_ptr<const Condition>> m_activationCondition;
    std::optional<std::shared_ptr<const Condition>> m_endCondition;

    std::optional<Qt::MouseButtons> m_mouseButtons;

    std::optional<Range<qreal>> m_threshold;
    bool m_withinThreshold = false;
    qreal m_absoluteAccumulatedDelta = 0;

    friend class TestTrigger;
};

}