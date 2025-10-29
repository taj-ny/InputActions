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

#include "Trigger.h"
#include <libinputactions/actions/InputAction.h>
#include <libinputactions/interfaces/InputEmitter.h>
#include <libinputactions/variables/VariableManager.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_TRIGGER, "inputactions.trigger", QtWarningMsg)

namespace InputActions
{

static const std::chrono::milliseconds TICK_INTERVAL{5L};

Trigger::Trigger(TriggerType type)
    : m_type(type)
{
    m_tickTimer.setTimerType(Qt::TimerType::PreciseTimer);
    connect(&m_tickTimer, &QTimer::timeout, this, &Trigger::onTick);
}

void Trigger::addAction(std::unique_ptr<TriggerAction> action)
{
    actionAdded(action.get());
    m_actions.push_back(std::move(action));
}

bool Trigger::canActivate(const TriggerActivationEvent &event) const
{
    if (!m_mouseButtons.empty() && event.mouseButtons.has_value()) {
        if (m_mouseButtons.size() != event.mouseButtons->size() || (m_mouseButtonsExactOrder && !std::ranges::equal(m_mouseButtons, event.mouseButtons.value()))
            || (!m_mouseButtonsExactOrder && !std::ranges::all_of(m_mouseButtons, [event](auto &button) {
                   return std::ranges::contains(event.mouseButtons.value(), button);
               }))) {
            return false;
        }
    }

    return !m_activationCondition || m_activationCondition->satisfied();
}

bool Trigger::canUpdate(const TriggerUpdateEvent &event) const
{
    return true;
}

bool Trigger::endIfCannotUpdate() const
{
    return false;
}

void Trigger::update(const TriggerUpdateEvent &event)
{
    m_absoluteAccumulatedDelta += std::abs(event.m_delta);
    m_withinThreshold = !m_threshold || m_threshold->contains(m_absoluteAccumulatedDelta);
    if (!m_withinThreshold) {
        qCDebug(INPUTACTIONS_TRIGGER).noquote() << QString("Threshold not reached (id: %1, current: %2, min: %3, max: %4")
                                                       .arg(m_id,
                                                            QString::number(m_absoluteAccumulatedDelta),
                                                            QString::number(m_threshold->min().value_or(-1)),
                                                            QString::number(m_threshold->max().value_or(-1)));
        return;
    }

    qCDebug(INPUTACTIONS_TRIGGER).noquote() << QString("Trigger updated (id: %1, delta: %2)").arg(m_id, QString::number(event.m_delta));

    if (!m_started) {
        qCDebug(INPUTACTIONS_TRIGGER).noquote() << QString("Trigger started (id: %1)").arg(m_id);
        m_started = true;
        m_tickTimer.start(TICK_INTERVAL);

        if (m_clearModifiers && *m_clearModifiers) {
            qCDebug(INPUTACTIONS_TRIGGER).noquote() << QString("Clearing keyboard modifiers (trigger: %1)").arg(m_id);
            g_inputEmitter->keyboardClearModifiers();
        }

        for (const auto &action : m_actions) {
            action->triggerStarted();
        }
    }

    setLastTrigger();
    updateActions(event);
}

bool Trigger::canEnd() const
{
    return m_withinThreshold && (!m_endCondition || m_endCondition->satisfied());
}

void Trigger::end()
{
    if (!m_started) {
        reset();
        return;
    }

    qCDebug(INPUTACTIONS_TRIGGER).noquote() << QString("Trigger ended (id: %1)").arg(m_id);
    setLastTrigger();
    for (const auto &action : m_actions) {
        action->triggerEnded();
    }
    Q_EMIT ended();
    reset();
}

void Trigger::cancel()
{
    if (!m_started) {
        reset();
        return;
    }

    qCDebug(INPUTACTIONS_TRIGGER).noquote() << QString("Trigger cancelled (id: %1)").arg(m_id);
    for (const auto &action : m_actions) {
        action->triggerCancelled();
    }
    reset();
}

bool Trigger::overridesOtherTriggersOnEnd()
{
    if (!m_withinThreshold) {
        return false;
    }

    return std::ranges::any_of(m_actions, [](const auto &action) {
        return (action->m_on == On::End || action->m_on == On::EndCancel) && action->canExecute();
    });
}

bool Trigger::overridesOtherTriggersOnUpdate()
{
    if (!m_withinThreshold) {
        return false;
    }

    return std::ranges::any_of(m_actions, [](const auto &action) {
        return action->action()->m_executions || (action->m_on == On::Update && action->canExecute());
    });
}

void Trigger::actionAdded(TriggerAction *action)
{
    if (dynamic_cast<const InputAction *>(action->action())) {
        if (!m_clearModifiers) {
            m_clearModifiers = true;
        }
    }
}

const std::vector<TriggerAction *> Trigger::actions()
{
    std::vector<TriggerAction *> result;
    for (auto &action : m_actions) {
        result.push_back(action.get());
    }
    return result;
}

void Trigger::updateActions(const TriggerUpdateEvent &event)
{
    for (const auto &action : m_actions) {
        action->triggerUpdated(event.m_delta, {});
    }
}

const TriggerType &Trigger::type() const
{
    return m_type;
}

void Trigger::onTick()
{
    if (!m_withinThreshold) {
        return;
    }

    for (const auto &action : m_actions) {
        action->triggerTick(TICK_INTERVAL.count());
    }
}

void Trigger::setLastTrigger()
{
    if (m_setLastTrigger) {
        g_variableManager->getVariable(BuiltinVariables::LastTriggerId)->set(m_id);
        g_variableManager->getVariable(BuiltinVariables::LastTriggerTimestamp)
            ->set(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());
    }
}

void Trigger::reset()
{
    m_started = false;
    m_absoluteAccumulatedDelta = 0;
    m_withinThreshold = false;
    m_tickTimer.stop();
}

}