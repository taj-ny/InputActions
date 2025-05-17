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

#include <libinputactions/globals.h>

#include <QString>

#include <any>

namespace libinputactions
{

class Variable;

class VariableOperationsBase
{
public:
    virtual ~VariableOperationsBase() = default;

    bool compare(const std::vector<std::any> &right, const ComparisonOperator &comparisonOperator) const;
    virtual bool compare(const std::any &left, const std::any &right, const ComparisonOperator &comparisonOperator) const;

    virtual QString toString(const std::any &value) const;

    static std::unique_ptr<VariableOperationsBase> create(Variable *variable);

protected:
    VariableOperationsBase(Variable *variable);

private:
    Variable *m_variable;
};

template<typename T>
class VariableOperations : public VariableOperationsBase
{
public:
    VariableOperations(Variable *variable);

    bool compare(const std::any &left, const std::any &right, const ComparisonOperator &comparisonOperator) const override;
    QString toString(const std::any &value) const override;

    static bool compare(const T &left, const T &right, const ComparisonOperator &comparisonOperator);
    static QString toString(const T &value);
};

}