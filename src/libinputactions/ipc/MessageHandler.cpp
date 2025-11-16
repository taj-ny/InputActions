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

#include "MessageHandler.h"
#include "messages.h"

namespace InputActions
{

void MessageHandler::handleMessage(std::shared_ptr<const Message> message)
{
    switch (message->type()) {
        case MessageType::BeginSessionRequest:
            beginSessionRequestMessage(std::dynamic_pointer_cast<const BeginSessionRequestMessage>(message));
            break;
        case MessageType::EnvironmentState:
            environmentStateMessage(std::dynamic_pointer_cast<const EnvironmentStateMessage>(message));
            break;
        case MessageType::HandshakeRequest:
            handshakeRequestMessage(std::dynamic_pointer_cast<const HandshakeRequestMessage>(message));
            break;
        case MessageType::InvokePlasmaGlobalShortcutRequest:
            invokePlasmaGlobalShortcutMessage(std::dynamic_pointer_cast<const InvokePlasmaGlobalShortcutRequestMessage>(message));
            break;
        case MessageType::LoadConfigRequest:
            loadConfigRequestMessage(std::dynamic_pointer_cast<const LoadConfigRequestMessage>(message));
            break;
        case MessageType::RecordStrokeRequest:
            recordStrokeRequestMessage(std::dynamic_pointer_cast<const RecordStrokeRequestMessage>(message));
            break;
        case MessageType::SendNotification:
            sendNotificationMessage(std::dynamic_pointer_cast<const SendNotificationMessage>(message));
            break;
        case MessageType::StartProcessRequestMessage:
            startProcessRequestMessage(std::dynamic_pointer_cast<const StartProcessRequestMessage>(message));
            break;
        case MessageType::VariableListRequest:
            variableListRequestMessage(std::dynamic_pointer_cast<const VariableListRequestMessage>(message));
            break;
    }
}

}