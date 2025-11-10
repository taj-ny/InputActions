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

#include "TriggerHandler.h"
#include <libinputactions/interfaces/InputEmitter.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_HANDLER_TRIGGER, "inputactions.handler.trigger", QtWarningMsg)

namespace InputActions
{

static const std::vector<TriggerType> TIMED_TRIGGERS = {TriggerType::Click, TriggerType::KeyboardShortcut, TriggerType::Hover, TriggerType::Press};

TriggerHandler::TriggerHandler()
{
    m_timedTriggerUpdateTimer.setTimerType(Qt::PreciseTimer);
    m_timedTriggerUpdateTimer.setInterval(m_timedTriggerUpdateDelta);
    connect(&m_timedTriggerUpdateTimer, &QTimer::timeout, this, [this] {
        updateTimedTriggers();
    });
}

void TriggerHandler::addTrigger(std::unique_ptr<Trigger> trigger)
{
    m_triggers.push_back(std::move(trigger));
}

void TriggerHandler::setTimedTriggerUpdateDelta(uint32_t value)
{
    m_timedTriggerUpdateDelta = value;
    m_timedTriggerUpdateTimer.setInterval(value);
}

TriggerManagementOperationResult TriggerHandler::activateTriggers(TriggerTypes types, const TriggerActivationEvent &event)
{
    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Triggers activating (types: " << types << ")";
    cancelTriggers(TriggerType::All);
    reset();

    Q_EMIT activatingTriggers(types);

    TriggerManagementOperationResult result{};
    for (auto &trigger : triggers(types, event)) {
        Q_EMIT activatingTrigger(trigger);
        Q_EMIT trigger->activated();
        m_activeTriggers.push_back(trigger);
        qCDebug(INPUTACTIONS_HANDLER_TRIGGER).noquote() << QString("Trigger activated (id: %1)").arg(trigger->m_id);

        result.success = true;
        result.block = result.block || trigger->m_blockEvents;
    }
    m_timedTriggerUpdateTimer.start();

    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Triggers activated (count: " << m_activeTriggers.size() << ")";
    return result;
}

TriggerManagementOperationResult TriggerHandler::activateTriggers(TriggerTypes types)
{
    auto event = createActivationEvent();
    return activateTriggers(types, *event.get());
}

TriggerManagementOperationResult TriggerHandler::updateTriggers(const std::map<TriggerType, const TriggerUpdateEvent *> &events)
{
    TriggerTypes types{};
    for (const auto &[type, _] : events) {
        types |= type;
    }

    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Updating gestures (types: " << types << ")";

    TriggerManagementOperationResult result{};
    for (auto it = m_activeTriggers.begin(); it != m_activeTriggers.end();) {
        auto trigger = *it;
        const auto &type = trigger->type();
        if (!(types & type)) {
            it++;
            continue;
        }

        const auto event = events.at(type);
        if (!trigger->canUpdate(*event)) {
            if (trigger->endIfCannotUpdate()) {
                trigger->end();
            } else {
                trigger->cancel();
            }
            it = m_activeTriggers.erase(it);
            continue;
        }

        result.success = true;
        result.block = result.block || trigger->m_blockEvents;
        trigger->update(*event);

        if (m_activeTriggers.size() > 1) {
            qCDebug(INPUTACTIONS_TRIGGER, "Cancelling conflicting triggers");
            if (trigger->overridesOtherTriggersOnUpdate()) {
                cancelTriggers(trigger);
                break;
            } else if (types & TriggerType::Stroke && hasActiveTriggers(TriggerType::Swipe)) { // TODO This should be in MotionTriggerHandler
                cancelTriggers(TriggerType::Swipe);
                break;
            }
        }

        it++;
    }
    return result;
}

TriggerManagementOperationResult TriggerHandler::updateTriggers(TriggerType type, const TriggerUpdateEvent &event)
{
    return updateTriggers({{type, &event}});
}

TriggerManagementOperationResult TriggerHandler::endTriggers(TriggerTypes types)
{
    TriggerManagementOperationResult result{};
    if (!hasActiveTriggers(types)) {
        return result;
    }

    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).nospace() << "Ending gestures (types: " << types << ")";

    Q_EMIT endingTriggers(types);

    for (auto it = m_activeTriggers.begin(); it != m_activeTriggers.end();) {
        auto trigger = *it;
        if (!(types & trigger->type())) {
            it++;
            continue;
        }

        result.success = true;
        result.block = result.block || trigger->m_blockEvents;

        it = m_activeTriggers.erase(it);
        if (!trigger->canEnd()) {
            trigger->cancel();
            continue;
        }

        // Ending a trigger will reset some stuff that is required for this method
        if (trigger->overridesOtherTriggersOnEnd()) {
            cancelTriggers(trigger);
            trigger->end();
            break;
        }

        trigger->end();
    }
    return result;
}

TriggerManagementOperationResult TriggerHandler::cancelTriggers(TriggerTypes types)
{
    TriggerManagementOperationResult result{};
    if (!hasActiveTriggers(types)) {
        return result;
    }

    Q_EMIT cancellingTriggers(types);

    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).nospace() << "Cancelling triggers (types: " << types << ")";
    for (auto it = m_activeTriggers.begin(); it != m_activeTriggers.end();) {
        auto trigger = *it;
        if (!(types & trigger->type())) {
            it++;
            continue;
        }

        result.success = true;
        result.block = result.block || trigger->m_blockEvents;

        trigger->cancel();
        it = m_activeTriggers.erase(it);
    }
    return result;
}

void TriggerHandler::cancelTriggers(Trigger *except)
{
    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Cancelling triggers (except: " << except->m_id << ")";
    for (auto it = m_activeTriggers.begin(); it != m_activeTriggers.end();) {
        auto gesture = *it;
        if (gesture != except) {
            gesture->cancel();
            it = m_activeTriggers.erase(it);
            continue;
        }
        it++;
    }
}

std::vector<Trigger *> TriggerHandler::triggers(TriggerTypes types, const TriggerActivationEvent &event)
{
    std::vector<Trigger *> result;
    for (auto &trigger : m_triggers) {
        if (!(types & trigger->type()) || !trigger->canActivate(event)) {
            continue;
        }
        result.push_back(trigger.get());
    }
    return result;
}

std::vector<Trigger *> TriggerHandler::activeTriggers(TriggerTypes types)
{
    std::vector<Trigger *> result;
    for (auto &trigger : m_activeTriggers) {
        if (!(types & trigger->type())) {
            continue;
        }
        result.push_back(trigger);
    }
    return result;
}

bool TriggerHandler::hasActiveTriggers(TriggerTypes types)
{
    if (types == TriggerType::All) {
        return !m_activeTriggers.empty();
    }
    return std::any_of(m_activeTriggers.begin(), m_activeTriggers.end(), [&types](const auto &trigger) {
        return types & trigger->type();
    });
}

void TriggerHandler::updateTimedTriggers()
{
    if (!hasActiveTriggers()) {
        m_timedTriggerUpdateTimer.stop();
        return;
    }

    std::map<TriggerType, const TriggerUpdateEvent *> events;
    for (const auto &type : TIMED_TRIGGERS) {
        auto *event = new TriggerUpdateEvent;
        event->m_delta = m_timedTriggerUpdateDelta;
        events[type] = event;
    }

    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).nospace() << "Event (type: Time, delta: " << m_timedTriggerUpdateDelta << ")";
    const auto hasTriggers = updateTriggers(events).success;
    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).nospace() << "Event processed (type: Time, hasTriggers: " << hasTriggers << ")";

    for (auto &[_, event] : events) {
        delete event;
    }
}

std::unique_ptr<TriggerActivationEvent> TriggerHandler::createActivationEvent() const
{
    return std::make_unique<TriggerActivationEvent>();
}

void TriggerHandler::reset() {}

}