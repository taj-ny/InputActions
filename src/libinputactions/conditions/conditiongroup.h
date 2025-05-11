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

#include <libinputactions/conditions/condition.h>

#include <memory>
#include <vector>

namespace libinputactions
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
    explicit ConditionGroup(const ConditionGroupMode &mode = ConditionGroupMode::All);

    void add(const std::shared_ptr<const Condition> &condition);

protected:
    bool satisfiedInternal() const override;

    std::vector<std::shared_ptr<const Condition>> m_conditions;
    ConditionGroupMode m_mode;
};

}