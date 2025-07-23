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
#include <QString>
#include <libinputactions/actions/TriggerAction.h>
#include <libinputactions/conditions/Condition.h>
#include <libinputactions/globals.h>
#include <set>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS_TRIGGER)

namespace libinputactions
{

/**
 * Unset optional fields are not checked by triggers.
 */
class TriggerActivationEvent
{
public:
    std::set<uint32_t> keyboardKeys;
    std::optional<std::vector<Qt::MouseButton>> mouseButtons;
};
class TriggerUpdateEvent
{
public:
    TriggerUpdateEvent() = default;
    virtual ~TriggerUpdateEvent() = default;

    qreal m_delta{};
};

/**
 * An input action that does not involve motion.
 *
 * Triggers are managed by a trigger handler.
 */
class Trigger : public QObject
{
    Q_OBJECT

public:
    Trigger(TriggerType type = TriggerType::None);
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
    virtual bool canActivate(const TriggerActivationEvent &event) const;

    /**
     * Called by the trigger handler before updating a trigger. If true is returned, that trigger will be cancelled.
     * @internal
     */
    virtual bool canUpdate(const TriggerUpdateEvent &event) const;
    /**
     * @internal
     */
    TEST_VIRTUAL void update(const TriggerUpdateEvent &event);

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
    void setClearModifiers(bool value);

    /**
     * @param value Whether to set last_trigger variables. Default: true
     */
    void setSetLastTrigger(bool value);

    const std::vector<Qt::MouseButton> &mouseButtons() const;
    /**
     * Only applies to mouse triggers.
     * @param buttons Mouse buttons that must be pressed before and during the trigger.
     */
    void setMouseButtons(const std::vector<Qt::MouseButton> &buttons);

    const bool &mouseButtonsExactOrder() const;
    void setMouseButtonsExactOrder(bool value);

    const QString &id() const;
    /**
     * @param name Must be unique.
     */
    void setId(const QString &value);

    const TriggerType &type() const;

protected:
    /**
     * Called when an action is added. May be used to change default behavior.
     */
    virtual void actionAdded(TriggerAction *action);

    const std::vector<TriggerAction *> actions();

    virtual void updateActions(const TriggerUpdateEvent &event);

private:
    void reset();

    QString m_id;
    TriggerType m_type{0};
    std::vector<std::unique_ptr<TriggerAction>> m_actions;
    bool m_started = false;
    std::optional<bool> m_clearModifiers;
    bool m_setLastTrigger = true;

    std::optional<std::shared_ptr<const Condition>> m_activationCondition;
    std::optional<std::shared_ptr<const Condition>> m_endCondition;

    std::vector<Qt::MouseButton> m_mouseButtons;
    bool m_mouseButtonsExactOrder{};

    std::optional<Range<qreal>> m_threshold;
    bool m_withinThreshold = false;
    qreal m_absoluteAccumulatedDelta = 0;

    friend class TestTrigger;
};

}