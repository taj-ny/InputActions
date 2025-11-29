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

#include "ActionExecutor.h"
#include "Action.h"

namespace InputActions
{

ActionExecutor::ActionExecutor()
{
    m_sharedActionThreadPool.setMaxThreadCount(1);
}

void ActionExecutor::execute(const std::shared_ptr<Action> &action, ActionExecutionArguments &&arguments)
{
    const auto execute = [action = action, executions = arguments.executions]() { // copy action in case config gets reloaded while actions are scheduled
        action->execute(executions);
    };
    action->aboutToExecute();
    switch (arguments.thread) {
        case ActionThread::Auto:
            if (action->async() || m_sharedActionThreadPool.activeThreadCount()) {
                m_sharedActionThreadPool.start(execute);
                break;
            }
            [[fallthrough]];
        case ActionThread::Current:
            execute();
            break;
        case ActionThread::Own:
            m_ownActionThreadPool.start(execute);
            break;
    }
}

void ActionExecutor::clearQueue()
{
    m_ownActionThreadPool.clear();
    m_sharedActionThreadPool.clear();
}

void ActionExecutor::waitForDone()
{
    m_ownActionThreadPool.waitForDone();
    m_sharedActionThreadPool.waitForDone();
}

}