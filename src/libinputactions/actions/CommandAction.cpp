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
#include <QProcess>
#include <libinputactions/variables/VariableManager.h>

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

void CommandAction::executeImpl()
{
    const auto command = m_command.get().value_or("");
    if (command.isEmpty()) {
        return;
    }

    auto *process = new QProcess;
    connect(process, &QProcess::finished, this, [process]() {
        process->deleteLater();
    });
    process->setProgram("/bin/sh");
    process->setArguments({"-c", command});
    g_variableManager->setProcessEnvironment(*process);
    process->start();
    if (m_wait) {
        process->waitForFinished();
    }
}

}