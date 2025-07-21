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

namespace libinputactions
{

static const std::vector<TriggerType> TIMED_TRIGGERS = {TriggerType::Click, TriggerType::Press};

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

void TriggerHandler::handleEvent(const KeyboardKeyEvent *event)
{
    // Lazy way of detecting modifier release during mouse gestures
    if (event->state()) {
        return;
    }
    endTriggers(TriggerType::All);
}

bool TriggerHandler::handleEvent(const InputEvent *event)
{
    switch (event->type()) {
        case InputEventType::KeyboardKey:
            handleEvent(static_cast<const KeyboardKeyEvent *>(event));
            break;
        default:
            return false;
    }
    return false;
}

void TriggerHandler::setTimedTriggerUpdateDelta(uint32_t value)
{
    m_timedTriggerUpdateDelta = value;
    m_timedTriggerUpdateTimer.setInterval(value);
}

void TriggerHandler::registerTriggerActivateHandler(TriggerType type, const std::function<void()> &func)
{
    m_triggerActivateHandlers[type] = func;
}

void TriggerHandler::registerTriggerEndHandler(TriggerType type, const std::function<void()> &func)
{
    m_triggerEndHandlers[type] = func;
}

void TriggerHandler::registerTriggerEndCancelHandler(TriggerType type, const std::function<void()> &func)
{
    m_triggerEndCancelHandlers[type] = func;
}

bool TriggerHandler::activateTriggers(TriggerTypes types, const TriggerActivationEvent *event)
{
    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Triggers activating (types: " << types << ")";
    cancelTriggers(TriggerType::All);
    reset();

    for (const auto &[type, handler] : m_triggerActivateHandlers) {
        if (!(types & type)) {
            continue;
        }
        handler();
    }

    for (auto &trigger : triggers(types, event)) {
        triggerActivating(trigger);
        m_activeTriggers.push_back(trigger);
        qCDebug(INPUTACTIONS_HANDLER_TRIGGER).noquote() << QString("Trigger activated (id: %1)").arg(trigger->id());
    }
    m_timedTriggerUpdateTimer.start();

    const auto triggerCount = m_activeTriggers.size();
    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Triggers activated (count: " << triggerCount << ")";
    return triggerCount != 0;
}

bool TriggerHandler::activateTriggers(TriggerTypes types)
{
    auto event = createActivationEvent();
    return activateTriggers(types, event.get());
}

bool TriggerHandler::updateTriggers(const std::map<TriggerType, const TriggerUpdateEvent *> &events)
{
    TriggerTypes types{};
    for (const auto &[type, _] : events) {
        types |= type;
    }

    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Updating gestures (types: " << types << ")";

    auto hasTriggers = false;
    for (auto it = m_activeTriggers.begin(); it != m_activeTriggers.end();) {
        auto trigger = *it;
        const auto &type = trigger->type();
        if (!(types & type)) {
            it++;
            continue;
        }

        const auto &event = events.at(type);
        if (!trigger->canUpdate(event)) {
            trigger->cancel();
            it = m_activeTriggers.erase(it);
            continue;
        }

        hasTriggers = true;
        trigger->update(event);

        if (!m_conflictsResolved && m_activeTriggers.size() > 1) {
            qCDebug(INPUTACTIONS_TRIGGER, "Cancelling conflicting triggers");
            m_conflictsResolved = true;
            if (trigger->overridesOtherTriggersOnUpdate()) {
                cancelTriggers(trigger);
                break;
            } else if (types & TriggerType::Stroke) { // TODO This should be in MotionTriggerHandler
                cancelTriggers(TriggerType::Swipe);
                break;
            }
        }

        it++;
    }
    return hasTriggers;
}

bool TriggerHandler::updateTriggers(TriggerType type, const TriggerUpdateEvent *event)
{
    return updateTriggers({{type, event}});
}

bool TriggerHandler::endTriggers(TriggerTypes types)
{
    if (!hasActiveTriggers(types)) {
        return false;
    }

    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).nospace() << "Ending gestures (types: " << types << ")";

    for (const auto &[type, handler] : m_triggerEndHandlers) {
        if (!(types & type)) {
            continue;
        }
        handler();
    }
    for (const auto &[type, handler] : m_triggerEndCancelHandlers) {
        if (!(types & type)) {
            continue;
        }
        handler();
    }

    for (auto it = m_activeTriggers.begin(); it != m_activeTriggers.end();) {
        auto trigger = *it;
        if (!(types & trigger->type())) {
            it++;
            continue;
        }

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
    return true;
}

bool TriggerHandler::cancelTriggers(TriggerTypes types)
{
    if (!hasActiveTriggers(types)) {
        return false;
    }

    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).nospace() << "Cancelling triggers (types: " << types << ")";
    for (auto it = m_activeTriggers.begin(); it != m_activeTriggers.end();) {
        auto trigger = *it;
        if (!(types & trigger->type())) {
            it++;
            continue;
        }

        trigger->cancel();
        it = m_activeTriggers.erase(it);
    }
    return true;
}

void TriggerHandler::cancelTriggers(Trigger *except)
{
    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Cancelling triggers (except: " << except->id() << ")";
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

std::vector<Trigger *> TriggerHandler::triggers(TriggerTypes types, const TriggerActivationEvent *event)
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
    const auto hasTriggers = updateTriggers(events);
    qCDebug(INPUTACTIONS_HANDLER_TRIGGER).nospace() << "Event processed (type: Time, hasTriggers: " << hasTriggers << ")";

    for (auto &[_, event] : events) {
        delete event;
    }
}

std::unique_ptr<TriggerActivationEvent> TriggerHandler::createActivationEvent() const
{
    return std::make_unique<TriggerActivationEvent>();
}

void TriggerHandler::triggerActivating(const Trigger *trigger)
{
}

void TriggerHandler::reset()
{
    m_conflictsResolved = false;
}

}