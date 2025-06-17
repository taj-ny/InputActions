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

#include "VariableOperations.h"

#include <any>
#include <typeindex>

#include <QString>

namespace libinputactions
{

class Variable
{
public:
    Variable(const std::type_index &type);
    virtual ~Variable() = default;

    /**
     * @return May be empty.
     */
    virtual std::any get() const;
    /**
     * @param value Must be the same as the variable's type or empty.
     */
    virtual void set(const std::any &value);

    /**
     * @return Operations for this variable's type.
     */
    const VariableOperationsBase *operations() const;

    const std::type_index &type() const;

private:
    std::type_index m_type;
    std::variant<bool, QString> m_value;
    std::unique_ptr<VariableOperationsBase> m_operations;
};

/**
 * A locally stored variable with instant access.
 */
class LocalVariable : public Variable
{
public:
    LocalVariable(const std::type_index &type);

    std::any get() const override;
    void set(const std::any &value) override;

private:
    std::any m_value;
};

/**
 * A variable whose value is calculated or fetched on demand. Variables with slow access are currently not supported.
 */
class RemoteVariable : public Variable
{
public:
    /**
     * @param getter Must always return the same type as the variable or empty.
     */
    RemoteVariable(const std::type_index &type, const std::function<void(std::any &value)> &getter);

    std::any get() const override;

private:
    std::function<void(std::any &value)> m_getter;
};

}