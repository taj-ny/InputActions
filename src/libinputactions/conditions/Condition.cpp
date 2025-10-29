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

#include "Condition.h"
#include <libinputactions/config/Config.h>
#include <libinputactions/interfaces/NotificationManager.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

ConditionEvaluationArguments::ConditionEvaluationArguments()
    : variableManager(g_variableManager.get())
{
}

bool Condition::satisfied(const ConditionEvaluationArguments &arguments)
{
    try {
        return evaluate(arguments);
    } catch (const std::exception &) {
        return false;
    }
}

bool Condition::evaluate(const ConditionEvaluationArguments &arguments)
{
    try {
        return evaluateImpl(arguments) == !m_negate;
    } catch (const std::exception &e) {
        qWarning(INPUTACTIONS).noquote() << "Failed to evaluate condition: " << e.what();
        if (g_config && g_config->m_sendNotificationOnError && !m_exceptionNotificationShown) {
            g_notificationManager->sendNotification("Failed to evaluate condition", e.what());
            m_exceptionNotificationShown = true;
        }

        throw;
    }
}

bool Condition::evaluateImpl(const ConditionEvaluationArguments &arguments)
{
    return true;
}

}