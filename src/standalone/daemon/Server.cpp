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

#include "Server.h"
#include <QCoreApplication>
#include <QLocalServer>
#include <QThread>
#include <libinputactions/ipc/MessageSocketConnection.h>
#include <sys/stat.h>

namespace InputActions
{

Server::~Server()
{
    removeSocket();
}

void Server::start()
{
    removeSocket();

    m_server = new QLocalServer(this);
    connect(m_server, &QLocalServer::newConnection, this, &Server::onNewConnection);

    m_server->listen(INPUTACTIONS_IPC_SOCKET_PATH);
    if (chmod(INPUTACTIONS_IPC_SOCKET_PATH.toStdString().c_str(), 0666) != 0) {
        qWarning(INPUTACTIONS_IPC) << "Failed to set socket permissions";
    }
}

void Server::onNewConnection()
{
    auto *qtSocket = m_server->nextPendingConnection();
    auto *socket = new MessageSocketConnection(qtSocket, this);

    connect(qtSocket, &QLocalSocket::disconnected, this, [socket]() {
        socket->deleteLater();
    });
    connect(socket, &MessageSocketConnection::messageReceived, this, [this](const auto &message) {
        Q_EMIT messageReceived(message);
    });
}

void Server::removeSocket()
{
    QLocalServer::removeServer(INPUTACTIONS_IPC_SOCKET_PATH);
}

}