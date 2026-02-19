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

#include <libinputactions/conditions/Condition.h>
#include <memory>
#include <vector>

namespace InputActions
{

enum class ConditionGroupMode
{
    All,
    Any,
    None
};

/**
 * Contains multiple conditions. Checks whether all, any or none of them are satisfied, depending on the specified
 * mode.
 */
class ConditionGroup : public Condition
{
public:
    ConditionGroup(ConditionGroupMode mode = ConditionGroupMode::All);

    ConditionGroupMode mode() const { return m_mode; }
    const std::vector<std::shared_ptr<Condition>> &conditions() const { return m_conditions; }

    void append(const std::shared_ptr<Condition> &condition);
    void prepend(const std::shared_ptr<Condition> &condition);

protected:
    bool evaluateImpl(const ConditionEvaluationArguments &arguments) override;

    std::vector<std::shared_ptr<Condition>> m_conditions;
    ConditionGroupMode m_mode;
};

}