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

#include <libinputactions/interfaces/implementations/DBusNotificationManager.h>
#include <libinputactions/interfaces/implementations/DBusPlasmaGlobalShortcutInvoker.h>
#include <libinputactions/interfaces/implementations/ProcessRunnerImpl.h>
#include <libinputactions/ipc/MessageHandler.h>

namespace InputActions
{

class Client;

class ClientMessageHandler
    : public QObject
    , public MessageHandler
{
    Q_OBJECT

public:
    ClientMessageHandler(Client *client);

protected:
    void invokePlasmaGlobalShortcutMessage(const std::shared_ptr<const InvokePlasmaGlobalShortcutRequestMessage> &message) override;
    void sendNotificationMessage(const std::shared_ptr<const SendNotificationMessage> &message) override;
    void startProcessRequestMessage(const std::shared_ptr<const StartProcessRequestMessage> &message) override;

private:
    DBusNotificationManager m_notificationManager;
    DBusPlasmaGlobalShortcutInvoker m_plasmaGlobalShortcutInvoker;
    ProcessRunnerImpl m_processRunner;
};

}