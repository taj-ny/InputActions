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

#include <libinputactions/actions/input.h>
#include <libinputactions/input/emitter.h>
#include <libinputactions/input/keyboard.h>

Q_LOGGING_CATEGORY(LIBINPUTACTIONS_TRIGGER, "libinputactions.trigger", QtWarningMsg)

namespace libinputactions
{

void Trigger::addAction(std::unique_ptr<TriggerAction> action)
{
    if (dynamic_cast<InputTriggerAction *>(action.get())) {
        if (!m_clearModifiers) {
            m_clearModifiers = true;
        }
    }

    m_actions.push_back(std::move(action));
}

void Trigger::setActivationCondition(const std::shared_ptr<const Condition> &condition)
{
    m_activationCondition = condition;
}

void Trigger::setEndCondition(const std::shared_ptr<const Condition> &condition)
{
    m_endCondition = condition;
}

bool Trigger::canActivate(const TriggerActivationEvent *event) const
{
    if (m_mouseButtons && event->mouseButtons && *m_mouseButtons != event->mouseButtons) {
        return false;
    }

    return !m_activationCondition || m_activationCondition.value()->satisfied();
}

bool Trigger::canUpdate(const TriggerUpdateEvent *) const
{
    return true;
}

void Trigger::update(const TriggerUpdateEvent *event)
{
    m_absoluteAccumulatedDelta += std::abs(event->delta());
    m_withinThreshold = !m_threshold || m_threshold->contains(m_absoluteAccumulatedDelta);
    if (!m_withinThreshold) {
        qCDebug(LIBINPUTACTIONS_TRIGGER).noquote()
            << QString("Threshold not reached (name: %1, current: %2, min: %3, max: %4")
                .arg(m_name, QString::number(m_absoluteAccumulatedDelta), QString::number(m_threshold->min().value_or(-1)), QString::number(m_threshold->max().value_or(-1)));
        return;
    }

    qCDebug(LIBINPUTACTIONS_TRIGGER).noquote() << QString("Trigger updated (name: %1, delta: %2)").arg(m_name, QString::number(event->delta()));

    if (!m_started) {
        qCDebug(LIBINPUTACTIONS_TRIGGER).noquote() << QString("Trigger started (name: %1)").arg(m_name);
        m_started = true;

        if (m_clearModifiers && *m_clearModifiers) {
            qCDebug(LIBINPUTACTIONS_TRIGGER).noquote() << QString("Clearing keyboard modifiers (trigger: %1)").arg(m_name);
            Keyboard::instance()->clearModifiers();
        }

        for (const auto &action : m_actions) {
            action->triggerStarted();
        }
    }

    updateActions(event);
}

bool Trigger::canEnd(const TriggerEndEvent *event) const
{
    return m_withinThreshold && (!m_endCondition || m_endCondition.value()->satisfied());
}

void Trigger::end()
{
    if (!m_started) {
        reset();
        return;
    }

    qCDebug(LIBINPUTACTIONS_TRIGGER).noquote() << QString("Trigger ended (name: %1)").arg(m_name);
    for (const auto &action : m_actions) {
        action->triggerEnded();
    }
    reset();
}

void Trigger::cancel()
{
    if (!m_started) {
        reset();
        return;
    }

    qCDebug(LIBINPUTACTIONS_TRIGGER).noquote() << QString("Trigger cancelled (name: %1)").arg(m_name);
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

    return std::any_of(m_actions.begin(), m_actions.end(), [](const auto &action) {
        return (action->on() == On::End || action->on() == On::EndCancel) && action->canExecute();
    });
}

bool Trigger::overridesOtherTriggersOnUpdate()
{
    if (!m_withinThreshold) {
        return false;
    }

    return std::any_of(m_actions.begin(), m_actions.end(), [](const auto &action) {
        return action->executed() || (action->on() == On::Update && action->canExecute());
    });
}

const std::vector<TriggerAction *> Trigger::actions()
{
    std::vector<TriggerAction *> result;
    for (auto &action : m_actions) {
        result.push_back(action.get());
    }
    return result;
}

void Trigger::updateActions(const TriggerUpdateEvent *event)
{
    for (const auto &action : m_actions) {
        action->triggerUpdated(event->delta(), {});
    }
}

void Trigger::setThreshold(const Range<qreal> &threshold)
{
    m_threshold = threshold;
}

void Trigger::setClearModifiers(const bool &value)
{
    m_clearModifiers = value;
}

const std::optional<Qt::MouseButtons> &Trigger::mouseButtons() const
{
    return m_mouseButtons;
}

void Trigger::setMouseButtons(const std::optional<Qt::MouseButtons> &buttons)
{
    m_mouseButtons = buttons;
}

const QString &Trigger::name() const
{
    return m_name;
}

void Trigger::setName(const QString &name)
{
    m_name = name;
}

const TriggerType &Trigger::type() const
{
    return m_type;
}

void Trigger::setType(const TriggerType &type)
{
    m_type = type;
}

void Trigger::reset()
{
    m_started = false;
    m_absoluteAccumulatedDelta = 0;
    m_withinThreshold = false;
}

const qreal &TriggerUpdateEvent::delta() const
{
    return m_delta;
}

void TriggerUpdateEvent::setDelta(const qreal &delta)
{
    m_delta = delta;
}

}