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

#include "ActionGroup.h"
#include "ActionExecutor.h"
#include <libinputactions/conditions/Condition.h>
#include <libinputactions/helpers/QThread.h>

namespace InputActions
{

ActionGroup::ActionGroup(ActionGroupExecutionMode mode)
    : m_mode(mode)
{
}

void ActionGroup::executeImpl(const ActionExecutionArguments &args)
{
    // TODO Each action introduces latency
    const auto checkCanExecute = [](const auto &action) {
        auto result = true;
        QThreadHelpers::runOnThread(
            QThreadHelpers::mainThread(),
            [&action, &result]() {
                if (!action->canExecute()) {
                    result = false;
                }
            },
            true);
        return result;
    };

    switch (m_mode) {
        case ActionGroupExecutionMode::All:
            for (const auto &action : m_actions) {
                if (!checkCanExecute(action)) {
                    continue;
                }
                g_actionExecutor->execute(*action,
                                          {
                                              .thread = ActionThread::Current,
                                              .actionArgs = args,
                                          });
            }
            break;
        case ActionGroupExecutionMode::First:
            for (const auto &action : m_actions) {
                if (checkCanExecute(action)) {
                    g_actionExecutor->execute(*action,
                                              {
                                                  .thread = ActionThread::Current,
                                                  .actionArgs = args,
                                              });
                    break;
                }
            }
            break;
    }
}

std::vector<const Action *> ActionGroup::actions() const
{
    std::vector<const Action *> result;
    for (const auto &action : m_actions) {
        result.push_back(action.get());
    }
    return result;
}

void ActionGroup::append(std::unique_ptr<Action> action)
{
    m_actions.push_back(std::move(action));
}

bool ActionGroup::async() const
{
    // this is wrong because of conditions but no one will notice
    return std::ranges::any_of(m_actions, [](const auto &action) {
        return action->async();
    });
}

void ActionGroup::reset()
{
    Action::reset();
    for (const auto &action : m_actions) {
        action->reset();
    }
}

}