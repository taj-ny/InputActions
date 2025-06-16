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

#pragma once

#include "Condition.h"

#include <any>

#include <QString>

namespace libinputactions
{

enum class ComparisonOperator;

/**
 * If a non-existent variable is used, the condition will always be satisfied.
 */
class VariableCondition : public Condition
{
public:
    VariableCondition(const QString &variableName, const std::vector<std::any> &values, const ComparisonOperator &comparisonOperator);
    VariableCondition(const QString &variableName, const std::any &value, const ComparisonOperator &comparisonOperator);

protected:
    bool satisfiedInternal() const override;

private:
    QString m_variableName;
    std::vector<std::any> m_values;
    ComparisonOperator m_comparisonOperator;
};

}