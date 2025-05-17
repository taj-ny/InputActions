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

#include "conditiongroup.h"

#include <algorithm>

namespace libinputactions
{

ConditionGroup::ConditionGroup(const ConditionGroupMode &mode)
    : m_mode(mode)
{
}

bool ConditionGroup::satisfiedInternal() const
{
    const auto begin = m_conditions.begin();
    const auto end = m_conditions.end();
    const auto pred = [](const auto &condition) {
        return condition->satisfied();
    };
    switch (m_mode)
    {
        case ConditionGroupMode::All:
            return std::all_of(begin, end, pred);
        case ConditionGroupMode::Any:
            return std::any_of(begin, end, pred);
        case ConditionGroupMode::None:
            return std::none_of(begin, end, pred);
        default:
            return false;
    }
}

void ConditionGroup::add(const std::shared_ptr<const Condition> &condition)
{
    m_conditions.push_back(condition);
}

}