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

#include "ActionGroup.h"
#include "ActionExecutor.h"
#include "InputActions.h"
#include <libinputactions/conditions/Condition.h>

namespace libinputactions
{

ActionGroup::ActionGroup(std::vector<std::shared_ptr<Action>> actions, ExecutionMode mode)
    : m_actions(std::move(actions))
    , m_mode(mode)
{
}

void ActionGroup::executeImpl()
{
    // TODO Each action with a condition introduces latency
    const auto evaluateCondition = [](const auto &action) {
        auto satisfied = true;
        if (const auto &condition = action->m_condition) {
            InputActions::runOnMainThread([&condition, &satisfied]() {
                if (!condition->satisfied()) {
                    satisfied = false;
                    return;
                }
            });
        }
        return satisfied;
    };

    switch (m_mode) {
        case ExecutionMode::All:
            for (const auto &action : m_actions) {
                if (!evaluateCondition(action)) {
                    continue;
                }
                g_actionExecutor->execute(action, ActionThread::Current);
            }
            break;
        case ExecutionMode::First:
            for (const auto &action : m_actions) {
                if (evaluateCondition(action)) {
                    g_actionExecutor->execute(action, ActionThread::Current);
                    break;
                }
            }
            break;
    }
}

bool ActionGroup::async() const
{
    // this is wrong because of conditions but no one will notice
    return std::ranges::any_of(m_actions, [](const auto &action) {
        return action->async();
    });
}

}