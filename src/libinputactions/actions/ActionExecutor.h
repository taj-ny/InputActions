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

#include <QThreadPool>

namespace libinputactions
{

class Action;

enum class ActionThread
{
    /**
     * If action is async, or is not but the action thread is busy, schedule it to execute on the action thread (shared between all actions). Otherwise, execute
     * it immediately. No actions will be executed until this one finishes.
     */
    Auto,
    /**
     * Execute the action on the current thread.
     */
    Current,
    /**
     * Execute the action on its own thread. Other actions will continue to be executed.
     */
    Own,
};

class ActionExecutor
{
public:
    ActionExecutor();

    /**
     * Executes an action without checking its condition.
     * @param thread Which thread to execute the action on.
     */
    void execute(const std::shared_ptr<Action> &action, ActionThread thread = ActionThread::Auto);

private:
    /**
     * Consists of one thread, shared across all actions.
     */
    QThreadPool m_sharedActionThreadPool;
    QThreadPool m_ownActionThreadPool;
};

inline std::unique_ptr<ActionExecutor> g_actionExecutor;

}