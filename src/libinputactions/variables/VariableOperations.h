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

#include <QString>
#include <any>
#include <libinputactions/globals.h>

namespace libinputactions
{

class Variable;

/**
 * Operations for variables of a specific type. Each variable has its own operations that contain a pointer to the variable.
 */
class VariableOperationsBase
{
public:
    virtual ~VariableOperationsBase() = default;

    /**
     * Compares the variables value to the specified value(s) using the specified operator.
     * @param right Must contain exactly 2 values if operator is Between. Must contain at least 1 value if operator is OneOf. All other operators require
     * exactly 1 value.
     */
    bool compare(const std::vector<std::any> &right, ComparisonOperator comparisonOperator) const;
    /**
     * @return A string representation of the variable's value or an empty string if not supported.
     */
    QString toString() const;
    /**
     * @return A string representation of the specified value of the same type as the variable or an empty string if not supported.
     */
    virtual QString toString(const std::any &value) const;

    static std::unique_ptr<VariableOperationsBase> create(Variable *variable);

protected:
    VariableOperationsBase(Variable *variable);

    /**
     * The operators NotEqualTo, OneOf and Between are not handled here.
     */
    virtual bool compare(const std::any &left, const std::any &right, ComparisonOperator comparisonOperator) const;

private:
    Variable *m_variable;
};

template<typename T>
class VariableOperations : public VariableOperationsBase
{
public:
    VariableOperations(Variable *variable);

    static bool compare(const T &left, const T &right, ComparisonOperator comparisonOperator);
    static QString toString(const T &value);
    QString toString(const std::any &value) const override;

protected:
    bool compare(const std::any &left, const std::any &right, ComparisonOperator comparisonOperator) const override;
};

}