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

#include "IPCProcessRunner.h"
#include "SessionManager.h"
#include <libinputactions/ipc/MessageSocketConnection.h>
#include <libinputactions/ipc/messages.h>

namespace InputActions
{

void IPCProcessRunner::startProcess(const QString &program, const QStringList &arguments, std::map<QString, QString> extraEnvironment, bool wait)
{
    StartProcessRequestMessage message;
    message.setProgram(program);
    message.setArguments(arguments);
    message.setEnvironment(extraEnvironment);
    message.setWait(wait);

    if (wait) {
        g_sessionManager->currentSession().client()->sendMessageAndWaitForResponse<ResponseMessage>(message);
    } else {
        g_sessionManager->currentSession().client()->sendMessage(message);
    }
}

QString IPCProcessRunner::startProcessReadOutput(const QString &program, const QStringList &arguments, std::map<QString, QString> extraEnvironment)
{
    StartProcessRequestMessage message;
    message.setProgram(program);
    message.setArguments(arguments);
    message.setEnvironment(extraEnvironment);
    message.setOutput(true);

    if (const auto reply = g_sessionManager->currentSession().client()->sendMessageAndWaitForResponse<ResponseMessage>(message)) {
        return reply->result();
    }
    return {};
}

}