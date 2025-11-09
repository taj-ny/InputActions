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

#include "ClientMessageHandler.h"
#include "Client.h"
#include <libinputactions/ipc/messages.h>

namespace InputActions
{

ClientMessageHandler::ClientMessageHandler(Client *client)
{
    connect(client, &Client::messageReceived, this, [this](const auto &message) {
        handleMessage(message);
    });
}

void ClientMessageHandler::sendNotificationMessage(const std::shared_ptr<const SendNotificationMessage> &message)
{
    m_notificationManager.sendNotification(message->title(), message->content());
}

void ClientMessageHandler::startProcessRequestMessage(const std::shared_ptr<const StartProcessRequestMessage> &message)
{
    StartProcessResponseMessage response;
    if (message->output()) {
        response.setOutput(m_processRunner.startProcessReadOutput(message->program(), message->arguments(), message->environment()));
    } else {
        m_processRunner.startProcess(message->program(), message->arguments(), message->environment(), message->wait());
    }
    message->reply(response);
}

}