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

#include "ConditionGroup.h"
#include <algorithm>

namespace libinputactions
{

ConditionGroup::ConditionGroup(ConditionGroupMode mode)
    : m_mode(mode)
{
}

ConditionEvaluationResult ConditionGroup::evaluateImpl()
{
    bool error{};
    const auto pred = [&error](auto &condition) {
        const auto result = condition->evaluate();
        if (result == ConditionEvaluationResult::Error) {
            error = true;
        }
        return result == ConditionEvaluationResult::Satisfied;
    };

    bool result{};
    switch (m_mode) {
        case ConditionGroupMode::All:
            result = std::ranges::all_of(m_conditions, pred);
            break;
        case ConditionGroupMode::Any:
            result = std::ranges::any_of(m_conditions, pred);
            if (error && result) {
                error = false;
            }
            break;
        case ConditionGroupMode::None:
            result = std::ranges::none_of(m_conditions, pred);
            break;
        default:
            return ConditionEvaluationResult::NotSatisfied;
    }
    if (error) {
        return ConditionEvaluationResult::Error;
    }
    return result ? ConditionEvaluationResult::Satisfied : ConditionEvaluationResult::NotSatisfied;
}

void ConditionGroup::add(const std::shared_ptr<Condition> &condition)
{
    m_conditions.push_back(condition);
}

}