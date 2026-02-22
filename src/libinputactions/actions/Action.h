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

#include <QString>
#include <memory>

namespace InputActions
{

class Condition;

/**
 * Actions must be executed using an ActionExecutor.
 */
class Action
{
public:
    Action();
    virtual ~Action();

    bool canExecute() const;
    /**
     * Called by ActionExecutor when the action is scheduled for execution at some point in the future. It is not guaranteed that the action will actually be
     * executed. May not be called from the main thread.
     */
    void aboutToExecute();
    /**
     * Do not call directly, use ActionExecutor instead.
     * @see executeImpl
     */
    void execute(uint32_t executions);
    /**
     * Whether the action should be executed asynchronously. A value of false does not guarantee that the action will be executed synchronously.
     * @see ActionExecutor::execute
     */
    virtual bool async() const;
    /**
     * Whether multiple executions of this action can be merged together.
     */
    virtual bool mergeable() const;

    virtual void reset();

    /**
     * Must be satisfied in order for the action to be executed. May be nullptr.
     */
    const std::shared_ptr<Condition> &condition() const { return m_condition; }
    void setCondition(std::shared_ptr<Condition> value) { m_condition = std::move(value); }

    /**
     * Must be unique.
     */
    const QString &id() const { return m_id; }
    void setId(QString value) { m_id = std::move(value); }

    /**
     * The maximum amount of times the action can execute during the gesture. 0 = no limit
     */
    void setExecutionLimit(uint32_t value) { m_executionLimit = value; }

    /**
     * Executions since last reset.
     */
    uint32_t executions() const { return m_executions; }

protected:
    /**
     * This method is not guaranteed to be called from the main thread.
     * @param executions If the action is mergeable, this is set to the intended execution count, otherwise 1. Must not be 0.
     */
    virtual void executeImpl(uint32_t executions) {}

private:
    std::shared_ptr<Condition> m_condition;
    QString m_id;
    uint32_t m_executions{};
    uint32_t m_executionLimit{};
};

}