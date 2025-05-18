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

#include "trigger.h"

#include <libinputactions/input/emitter.h>
#include <libinputactions/input/keyboard.h>

Q_LOGGING_CATEGORY(LIBINPUTACTIONS_HANDLER_TRIGGER, "libinputactions.handler.trigger", QtWarningMsg)

namespace libinputactions
{

TriggerHandler::TriggerHandler()
{
    m_pressTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_pressTimer, &QTimer::timeout, this, [this] {
        pressUpdate(s_pressDelta);
    });

    registerTriggerActivateHandler(TriggerType::Press, std::bind(&TriggerHandler::pressTriggerActivateHandler, this));
    registerTriggerEndCancelHandler(TriggerType::Press, std::bind(&TriggerHandler::pressTriggerEndCancelHandler, this));
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

void TriggerHandler::registerTriggerActivateHandler(const TriggerType &type, const std::function<void()> &func)
{
    m_triggerActivateHandlers[type] = func;
}

void TriggerHandler::registerTriggerEndHandler(const TriggerType &type, const std::function<void()> &func)
{
    m_triggerEndHandlers[type] = func;
}

void TriggerHandler::registerTriggerEndCancelHandler(const TriggerType &type, const std::function<void()> &func)
{
    m_triggerEndCancelHandlers[type] = func;
}

bool TriggerHandler::activateTriggers(const TriggerTypes &types, const TriggerActivationEvent *event)
{
    qCDebug(LIBINPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Triggers activating (types: " << types << ", fingers: " << event->fingers << ", mouseButtons: " << event->mouseButtons << ", keyboardModifiers: " << event->keyboardModifiers << ", position: " << event->position << ")";
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
        qCDebug(LIBINPUTACTIONS_HANDLER_TRIGGER).noquote() << QString("Trigger activated (name: %1)").arg(trigger->name());
    }
    const auto triggerCount = m_activeTriggers.size();
    qCDebug(LIBINPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Triggers activated (count: " << triggerCount << ")";
    return triggerCount != 0;
}

bool TriggerHandler::activateTriggers(const TriggerTypes &types, const uint8_t &fingers)
{
    auto event = createActivationEvent();
    event->fingers = fingers;
    return activateTriggers(types, event.get());
}

bool TriggerHandler::activateTriggers(const TriggerTypes &types)
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

    qCDebug(LIBINPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Updating gestures (types: " << types << ")";

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
            qCDebug(LIBINPUTACTIONS_TRIGGER, "Cancelling conflicting triggers");
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

bool TriggerHandler::updateTriggers(const TriggerType &type, const TriggerUpdateEvent *event)
{
    return updateTriggers({ {type, event } });
}

bool TriggerHandler::endTriggers(const TriggerTypes &types)
{
    if (!hasActiveTriggers(types)) {
        return false;
    }

    qCDebug(LIBINPUTACTIONS_HANDLER_TRIGGER).nospace() << "Ending gestures (types: " << types << ")";

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

bool TriggerHandler::cancelTriggers(const TriggerTypes &types)
{
    if (!hasActiveTriggers(types)) {
        return false;
    }

    qCDebug(LIBINPUTACTIONS_HANDLER_TRIGGER).nospace() << "Cancelling triggers (types: " << types << ")";
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
    qCDebug(LIBINPUTACTIONS_HANDLER_TRIGGER).noquote().nospace() << "Cancelling triggers (except: " << except->name() << ")";
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

std::vector<Trigger *> TriggerHandler::triggers(const TriggerTypes &types, const TriggerActivationEvent *event)
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

std::vector<Trigger *> TriggerHandler::activeTriggers(const TriggerTypes &types)
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

bool TriggerHandler::hasActiveTriggers(const TriggerTypes &types)
{
    return std::any_of(m_activeTriggers.begin(), m_activeTriggers.end(), [&types](const auto &trigger) {
        return types & trigger->type();
    });
}

void TriggerHandler::pressUpdate(const qreal &delta)
{
    if (!hasActiveTriggers(TriggerType::Press)) {
        return;
    }

    qCDebug(LIBINPUTACTIONS_HANDLER_TRIGGER).nospace() << "Event (type: Press, delta: " << delta << ")";
    TriggerUpdateEvent event;
    event.setDelta(delta);
    const auto hasGestures = updateTriggers(TriggerType::Press, &event);
    qCDebug(LIBINPUTACTIONS_HANDLER_TRIGGER).nospace() << "Event processed (type: Pinch, hasGestures: " << hasGestures << ")";
}

std::unique_ptr<TriggerActivationEvent> TriggerHandler::createActivationEvent() const
{
    auto event = std::make_unique<TriggerActivationEvent>();
    event->keyboardModifiers = Keyboard::instance()->modifiers();
    return event;
}

void TriggerHandler::triggerActivating(const Trigger *trigger)
{
}

void TriggerHandler::reset()
{
    m_conflictsResolved = false;
}

void TriggerHandler::pressTriggerActivateHandler()
{
    qCDebug(LIBINPUTACTIONS_HANDLER_TRIGGER) << "Starting press timer";
    m_pressTimer.start(s_pressDelta);
}

void TriggerHandler::pressTriggerEndCancelHandler()
{
    qCDebug(LIBINPUTACTIONS_HANDLER_TRIGGER) << "Stopping press timer";
    m_pressTimer.stop();
}

}