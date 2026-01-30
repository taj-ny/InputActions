/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

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

#include "Variable.h"

namespace InputActions
{

/**
 * A variable whose value is calculated or fetched on demand. Variables with slow access are currently not supported.
 */
class RemoteVariable : public Variable
{
public:
    /**
     * @param getter Must always return the same type as the variable or empty.
     */
    RemoteVariable(std::type_index type, std::function<void(std::any &value)> getter);

    std::any get() const override;

private:
    std::function<void(std::any &value)> m_getter;
};

}