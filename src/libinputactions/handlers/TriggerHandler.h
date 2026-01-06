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

#include <QTimer>
#include <libinputactions/triggers/Trigger.h>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS_HANDLER_TRIGGER)

namespace InputActions
{

struct TriggerManagementOperationResult
{
    /**
     * Whether the operation was performed on at least one trigger.
     */
    bool success{};
    /**
     * Whether the event corresponding to the operation should be blocked, if possible.
     */
    bool block{};
};

/**
 * Base class of all handlers.
 */
class TriggerHandler : public QObject
{
    Q_OBJECT

public:
    void addTrigger(std::unique_ptr<Trigger> trigger);

    /**
     * @param value The interval (in milliseconds) and delta used for updating time-based triggers.
     */
    void setTimedTriggerUpdateDelta(uint32_t value);

protected:
    TriggerHandler();

    /**
     * Cancels all active triggers and activates triggers of the specified types eligible for activation.
     */
    TriggerManagementOperationResult activateTriggers(TriggerTypes types, const TriggerActivationEvent &event);
    /**
     * @see activateTriggers(const TriggerTypes &, const TriggerActivationEvent *)
     */
    TriggerManagementOperationResult activateTriggers(TriggerTypes types);

    /**
     * Updates triggers of multiple types in order as added to the handler.
     */
    TriggerManagementOperationResult updateTriggers(const std::map<TriggerType, const TriggerUpdateEvent *> &events);
    /**
     * Updates triggers of a single type.
     * @warning Do not use this to update multiple trigger types, as it will prevent conflict resolution from working
     * correctly.
     * @see updateTriggers(const std::map<TriggerType, const TriggerUpdateEvent *> &events)
     */
    TriggerManagementOperationResult updateTriggers(TriggerType type, const TriggerUpdateEvent &event = {});

    /**
     * Ends the specified types of triggers.
     */
    TriggerManagementOperationResult endTriggers(TriggerTypes types);

    /**
     * Cancels the specified types of triggers.
     */
    TriggerManagementOperationResult cancelTriggers(TriggerTypes types);
    /**
     * Cancels all triggers leaving only the specified one.
     */
    void cancelTriggers(Trigger *except);

    /**
     * @return Triggers of the specified types eligible for activation.
     */
    std::vector<Trigger *> triggers(TriggerTypes types, const TriggerActivationEvent &event);
    /**
     * @return Blocking triggers of the specified types eligible for activation.
     */
    std::vector<Trigger *> blockingTriggers(TriggerTypes types, const TriggerActivationEvent &event);
    /**
     * @return Active triggers of the specified types.
     */
    std::vector<Trigger *> activeTriggers(TriggerTypes types);
    /**
     * @return Whether there are any triggers of the specified types.
     */
    bool hasActiveTriggers(TriggerTypes types = TriggerType::All);
    /**
     * @return Whether there are any blocking triggers of the specified types.
     */
    bool hasActiveBlockingTriggers(TriggerTypes types = TriggerType::All);

    /**
     * Creates a trigger activation event with information that can be provided by the input device(s).
     * This implementation sets keyboard modifiers.
     */
    virtual std::unique_ptr<TriggerActivationEvent> createActivationEvent() const;

    /**
     * Resets member variables that hold information about the performed input action.
     */
    virtual void reset();

    void updateTimedTriggers();

signals:
    void activatingTrigger(const Trigger *trigger);
    void activatingTriggers(TriggerTypes types);
    void cancellingTriggers(TriggerTypes types);
    void endingTriggers(TriggerTypes types);

private:
    /**
     * Updates timed triggers. Stops itself if no triggers are active.
     */
    QTimer m_timedTriggerUpdateTimer;
    uint32_t m_timedTriggerUpdateDelta = 5;

    std::vector<std::unique_ptr<Trigger>> m_triggers;
    std::vector<Trigger *> m_activeTriggers;

    friend class TestTriggerHandler;
};

}