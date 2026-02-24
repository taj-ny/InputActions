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

#include "Action.h"
#include <vector>

namespace InputActions
{

enum class ActionGroupExecutionMode
{
    /**
     * Executes all conditions that satisfy their condition.
     */
    All,
    /**
     * Executes the first action that satisfied its condition.
     */
    First,
};

/**
 * Executes a set of actions in a specific way.
 */
class ActionGroup : public Action
{
public:
    ActionGroup(ActionGroupExecutionMode mode = ActionGroupExecutionMode::All);

    std::vector<const Action *> actions() const;
    void append(std::unique_ptr<Action> action);
    void setActions(std::vector<std::unique_ptr<Action>> actions) { m_actions = std::move(actions); }

    ActionGroupExecutionMode mode() const { return m_mode; }

    bool async() const override;

    void reset() override;

protected:
    void executeImpl(const ActionExecutionArguments &args) override;

private:
    std::vector<std::unique_ptr<Action>> m_actions;
    ActionGroupExecutionMode m_mode;
};

}