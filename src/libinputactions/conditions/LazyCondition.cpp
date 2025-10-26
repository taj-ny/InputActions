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

#include "LazyCondition.h"
#include <QRegularExpression>
#include <libinputactions/config/Config.h>
#include <libinputactions/globals.h>
#include <libinputactions/interfaces/NotificationManager.h>

namespace libinputactions
{

LazyCondition::LazyCondition(std::function<std::shared_ptr<Condition>()> constructor, QString errorMessage)
    : m_constructor(std::move(constructor))
    , m_errorMessage(std::move(errorMessage))
{
}

ConditionEvaluationResult LazyCondition::evaluateImpl()
{
    if (m_constructor) {
        m_condition = m_constructor();
    }
    if (!m_condition) {
        qWarning(INPUTACTIONS).noquote() << m_errorMessage;
        if (g_config && g_config->m_sendNotificationOnError && !m_errorNotificationShown) {
            g_notificationManager->sendNotification("Failed to evaluate condition", m_errorMessage);
            m_errorNotificationShown = true;
        }
        return ConditionEvaluationResult::Error;
    }

    m_constructor = {};
    return m_condition->evaluate();
}

}