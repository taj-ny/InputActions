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

#include <QString>
#include <memory>

namespace libinputactions
{

class Condition;

class Action
{
public:
    Action();
    virtual ~Action();

    bool canExecute() const;
    /**
     * Do not call directly, use ActionExecutor instead.
     * @see executeImpl
     */
    void execute();
    /**
     * Whether the action should be executed asynchronously. A value of false does not guarantee that the action will be executed synchronously.
     * @see ActionExecutor::execute
     */
    virtual bool async() const;

    void reset();

    /**
     * Must be satisfied in order for the action to be executed.
     */
    std::shared_ptr<Condition> m_condition;
    /**
     * Must be unique.
     */
    QString m_id;
    /**
     * Executions since last reset.
     */
    uint32_t m_executions{};

protected:
    /**
     * This method is not guaranteed to be called from the main thread. Implementations should use InputActions::runOnMainThread to schedule code to run on the
     * main thread.
     */
    virtual void executeImpl() {};
};

}