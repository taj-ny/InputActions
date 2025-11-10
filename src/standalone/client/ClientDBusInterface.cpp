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

#include "ClientDBusInterface.h"
#include "Client.h"
#include <libinputactions/dbus/IntegratedDBusInterface.h>
#include <libinputactions/ipc/MessageSocketConnection.h>

namespace InputActions
{

static const QString ERROR_NO_REPLY = "Daemon did not reply";

ClientDBusInterface::ClientDBusInterface(Client *client)
    : m_client(client)
    , m_bus(QDBusConnection::sessionBus())
{
    connect(client, &Client::connected, this, &ClientDBusInterface::onClientConnected);

    m_bus.registerService(INPUTACTIONS_DBUS_SERVICE);
    m_bus.registerObject(INPUTACTIONS_DBUS_PATH, this, QDBusConnection::ExportAllContents);
}

ClientDBusInterface::~ClientDBusInterface()
{
    m_bus.unregisterService(INPUTACTIONS_DBUS_SERVICE);
    m_bus.unregisterObject(INPUTACTIONS_DBUS_PATH);
}

void ClientDBusInterface::environmentState(QString state)
{
    EnvironmentStateMessage message;
    message.setStateJson(state);
    m_client->socketConnection()->sendMessage(message);
}

QString ClientDBusInterface::recordStroke()
{
    RecordStrokeRequestMessage request;
    if (const auto response = m_client->socketConnection()->sendMessageAndWaitForResponse<RecordStrokeResponseMessage>(request)) {
        return response->success() ? response->stroke() : response->error();
    }
    return ERROR_NO_REPLY;
}

QString ClientDBusInterface::reloadConfig()
{
    LoadConfigRequestMessage request;
    request.setConfig(m_client->configProvider.currentConfig());
    if (const auto response = m_client->socketConnection()->sendMessageAndWaitForResponse<LoadConfigResponseMessage>(request)) {
        return response->success() ? "success" : response->error();
    }
    return ERROR_NO_REPLY;
}

QString ClientDBusInterface::variables(QString filter)
{
    VariableListRequestMessage request;
    request.setFilter(filter);
    if (const auto response = m_client->socketConnection()->sendMessageAndWaitForResponse<VariableListResponseMessage>(request)) {
        return response->variables();
    }
    return ERROR_NO_REPLY;
}

void ClientDBusInterface::onClientConnected()
{
    Q_EMIT environmentStateRequested();
}

}