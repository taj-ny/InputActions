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

#include "VariableCondition.h"
#include <QLoggingCategory>
#include <QRegularExpression>
#include <libinputactions/variables/Variable.h>
#include <libinputactions/variables/VariableManager.h>

Q_LOGGING_CATEGORY(INPUTACTIONS_CONDITION_VARIABLE, "inputactions.condition.variable", QtWarningMsg)

namespace libinputactions
{

VariableCondition::VariableCondition(const QString &variableName, const std::vector<Value<std::any>> &values, ComparisonOperator comparisonOperator)
    : m_variableName(variableName)
    , m_values(values)
    , m_comparisonOperator(comparisonOperator)
{
}

VariableCondition::VariableCondition(const QString &variableName, const Value<std::any> &value, ComparisonOperator comparisonOperator)
    : VariableCondition(variableName, std::vector<Value<std::any>>{value}, comparisonOperator)
{
}

bool VariableCondition::evaluateImpl(const ConditionEvaluationArguments &arguments)
{
    const auto variable = arguments.variableManager->getVariable(m_variableName);
    if (!variable) {
        throw std::runtime_error(std::format("Variable {} does not exist.", m_variableName.toStdString()));
    }

    std::vector<std::any> values;
    for (const auto &valueProvider : m_values) {
        if (const auto value = valueProvider.get()) {
            values.push_back(value.value());
        } else {
            return false;
        }
    }
    return variable->operations()->compare(values, m_comparisonOperator);
}

}