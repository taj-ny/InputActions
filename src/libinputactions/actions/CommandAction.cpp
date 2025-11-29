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

#include "CommandAction.h"
#include <libinputactions/interfaces/ProcessRunner.h>

namespace InputActions
{

CommandAction::CommandAction(Value<QString> command)
    : m_command(std::move(command))
{
}

bool CommandAction::async() const
{
    return m_wait || m_command.expensive();
}

void CommandAction::executeImpl(uint32_t executions)
{
    const auto command = m_command.get().value_or("");
    if (command.isEmpty()) {
        return;
    }

    g_processRunner->startProcess("/bin/sh", {"-c", command}, m_wait);
}

}