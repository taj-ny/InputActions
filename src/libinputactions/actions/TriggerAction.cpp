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

#include "ActionExecutor.h"
#include "InputAction.h"
#include "TriggerAction.h"

Q_LOGGING_CATEGORY(INPUTACTIONS_ACTION, "inputactions.action", QtWarningMsg)

namespace libinputactions
{

TriggerAction::TriggerAction(std::shared_ptr<Action> action)
    : m_action(std::move(action))
{
}

TriggerAction::~TriggerAction() = default;

void TriggerAction::triggerStarted()
{
    m_executed = false;
    if (m_on == On::Begin) {
        tryExecute();
    }

    reset();
}

void TriggerAction::triggerUpdated(qreal delta, const QPointF &deltaPointMultiplied)
{
    if (!delta) {
        return;
    }

    if (auto *inputAction = dynamic_cast<InputAction *>(m_action.get())) {
        inputAction->m_deltaMultiplied = deltaPointMultiplied;
    }
    if (std::signbit(m_accumulatedDelta) != std::signbit(delta)) {
        // Direction changed
        m_accumulatedDelta = delta;
        qCDebug(INPUTACTIONS_ACTION).noquote() << QString("Gesture direction changed (name: %1)").arg(m_name);
    } else {
        m_accumulatedDelta += delta;
        m_absoluteAccumulatedDelta += std::abs(delta);
    }
    qCDebug(INPUTACTIONS_ACTION()).noquote() << QString("Action updated (name: %1, accumulatedDelta: %2)").arg(m_name, QString::number(m_accumulatedDelta));

    if (m_on != On::Update) {
        return;
    }
    const auto interval = m_interval.value();
    if (interval == 0 && m_interval.matches(delta)) {
        tryExecute();
        return;
    }

    // Keep executing action until accumulated delta no longer exceeds the interval
    while (m_interval.matches(m_accumulatedDelta) && std::abs(m_accumulatedDelta / interval) >= 1) {
        tryExecute();
        if (std::signbit(m_accumulatedDelta) != std::signbit(interval)) {
            m_accumulatedDelta += interval;
        } else {
            m_accumulatedDelta -= interval;
        }
    }
}

void TriggerAction::triggerEnded()
{
    if (m_on == On::End || m_on == On::EndCancel) {
        tryExecute();
    }

    m_executed = false;
    reset();
}

void TriggerAction::triggerCancelled()
{
    if (m_on == On::Cancel || m_on == On::EndCancel) {
        tryExecute();
    }

    m_executed = false;
    reset();
}

void TriggerAction::tryExecute()
{
    if (!canExecute()) {
        return;
    }

    qCDebug(INPUTACTIONS_ACTION).noquote() << QString("Action executed (name: %1)").arg(m_name);
    g_actionExecutor->execute(m_action);
    m_executed = true;
}

bool TriggerAction::canExecute() const
{
    return (!m_action->m_condition || m_action->m_condition->satisfied()) && (!m_threshold || m_threshold->contains(m_absoluteAccumulatedDelta));
}

void TriggerAction::reset()
{
    m_accumulatedDelta = 0;
    m_absoluteAccumulatedDelta = 0;
}

const bool &TriggerAction::executed() const
{
    return m_executed;
}

const QString &TriggerAction::name() const
{
    return m_name;
}

void TriggerAction::setName(const QString &name)
{
    m_name = name;
}

const On &TriggerAction::on() const
{
    return m_on;
}

void TriggerAction::setRepeatInterval(const ActionInterval &interval)
{
    m_interval = interval;
}

void TriggerAction::setThreshold(const Range<qreal> &threshold)
{
    m_threshold = threshold;
}

void TriggerAction::setOn(On on)
{
    m_on = on;
}

const Action *TriggerAction::action() const
{
    return m_action.get();
}

bool ActionInterval::matches(qreal value) const
{
    if (m_direction == IntervalDirection::Any) {
        return true;
    } else {
        return (value < 0 && m_direction == IntervalDirection::Negative) || (value > 0 && m_direction == IntervalDirection::Positive);
    }
}

const qreal &ActionInterval::value() const
{
    return m_value;
}

void ActionInterval::setValue(qreal value)
{
    m_value = value;
}

void ActionInterval::setDirection(const IntervalDirection &direction)
{
    m_direction = direction;
}

}