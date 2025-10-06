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
#include <QString>
#include <any>
#include <typeindex>

namespace libinputactions
{

class Variable
{
public:
    Variable(std::type_index type);
    virtual ~Variable() = default;

    /**
     * @return May be empty.
     */
    virtual std::any get() const { return {}; };
    /**
     * @param value Must be the same as the variable's type or empty.
     */
    virtual void set(std::any value) {};

    /**
     * @return Operations for this variable's type.
     */
    const VariableOperationsBase *operations() const;

    const std::type_index &type() const;

    /**
     * Whether the value should not be shown in the DBus interface.
     */
    bool m_hidden{};

private:
    std::type_index m_type;
    std::variant<bool, QString> m_value;
    std::unique_ptr<VariableOperationsBase> m_operations;
};

}