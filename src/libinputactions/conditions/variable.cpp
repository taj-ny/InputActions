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

#include "variable.h"
#include "conditiongroup.h"

#include <libinputactions/variable.h>

#include <QLoggingCategory>
#include <QRegularExpression>

Q_LOGGING_CATEGORY(LIBINPUTACTIONS_CONDITION_VARIABLE, "libinputactions.condition.variable", QtWarningMsg)

namespace libinputactions
{

VariableCondition::VariableCondition(const QString &variableName, const std::vector<std::any> &values, const ComparisonOperator &comparisonOperator)
    : m_variableName(variableName)
    , m_values(values)
    , m_comparisonOperator(comparisonOperator)
{
}

VariableCondition::VariableCondition(const QString &variableName, const std::any &value, const ComparisonOperator &comparisonOperator)
    : VariableCondition(variableName, std::vector<std::any>{value}, comparisonOperator)
{
}

bool VariableCondition::satisfiedInternal() const
{
    const auto variable = VariableManager::instance()->getVariable(m_variableName);
    if (!variable) {
        qCWarning(LIBINPUTACTIONS_CONDITION_VARIABLE).noquote()
            << QString("Variable \"%1\" is unsupported or doesn't exist, assuming the condition is satisfied.")
                .arg(m_variableName);
        return true;
    }
    return variable->compare(m_values, m_comparisonOperator);
}

}