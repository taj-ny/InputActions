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

#include "Action.h"
#include <libinputactions/conditions/Condition.h>
#include <libinputactions/globals.h>

namespace InputActions
{

Action::Action() = default;
Action::~Action() = default;

bool Action::canExecute() const
{
    return (!m_condition || m_condition->satisfied()) && (!m_executionLimit || m_executions < m_executionLimit);
}

void Action::aboutToExecute()
{
    m_executions++;
}

void Action::execute(const ActionExecutionArguments &args)
{
    qCDebug(INPUTACTIONS) << QString("Executing action \"%1\"").arg(m_id);
    executeImpl(args);
}

bool Action::async() const
{
    return false;
}

bool Action::mergeable() const
{
    return false;
}

void Action::reset()
{
    m_executions = 0;
}

}