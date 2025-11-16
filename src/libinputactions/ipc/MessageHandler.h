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

#include <memory>

namespace InputActions
{

class Message;
class BeginSessionRequestMessage;
class EnvironmentStateMessage;
class HandshakeRequestMessage;
class InvokePlasmaGlobalShortcutRequestMessage;
class LoadConfigRequestMessage;
class RecordStrokeRequestMessage;
class SendNotificationMessage;
class StartProcessRequestMessage;
class VariableListRequestMessage;

class MessageHandler
{
public:
    MessageHandler() = default;
    virtual ~MessageHandler() = default;

    void handleMessage(std::shared_ptr<const Message> message);

protected:
    virtual void beginSessionRequestMessage(const std::shared_ptr<const BeginSessionRequestMessage> &message) {}
    virtual void environmentStateMessage(const std::shared_ptr<const EnvironmentStateMessage> &message) {}
    virtual void handshakeRequestMessage(const std::shared_ptr<const HandshakeRequestMessage> &message) {}
    virtual void invokePlasmaGlobalShortcutMessage(const std::shared_ptr<const InvokePlasmaGlobalShortcutRequestMessage> &message) {}
    virtual void loadConfigRequestMessage(const std::shared_ptr<const LoadConfigRequestMessage> &message) {}
    virtual void recordStrokeRequestMessage(const std::shared_ptr<const RecordStrokeRequestMessage> &message) {}
    virtual void sendNotificationMessage(const std::shared_ptr<const SendNotificationMessage> &message) {}
    virtual void startProcessRequestMessage(const std::shared_ptr<const StartProcessRequestMessage> &message) {}
    virtual void variableListRequestMessage(const std::shared_ptr<const VariableListRequestMessage> &message) {}
};

}