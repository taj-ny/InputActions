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

#include "Client.h"
#include "ClientDBusInterface.h"
#include <QCoreApplication>
#include <libinputactions/InputActions.h>
#include <libinputactions/globals.h>
#include <libinputactions/ipc/MessageSocketConnection.h>
#include <libinputactions/ipc/messages.h>
#include <libinputactions/utils/SessionUtils.h>
#include <libinputactions/utils/ThreadUtils.h>

namespace InputActions
{

Client::Client()
    : m_dbusInterface(this)
    , m_currentTty(SessionUtils::currentTty())
{
}

Client::~Client() = default;

void Client::start()
{
    m_connectionRetryTimer = new QTimer(this);
    connect(m_connectionRetryTimer, &QTimer::timeout, this, &Client::connectToDaemon);
    m_connectionRetryTimer->setInterval(1000);

    auto socket = new QLocalSocket(this);
    m_connection = new MessageSocketConnection(socket, this);

    connect(&configProvider, &ConfigProvider::configChanged, this, &Client::onConfigChanged);
    connect(socket, &QLocalSocket::connected, this, &Client::onConnected);
    connect(socket, &QLocalSocket::errorOccurred, this, &Client::onErrorOccurred);
    connect(m_connection, &MessageSocketConnection::messageReceived, this, &Client::messageReceived);
    socket->connectToServer(INPUTACTIONS_IPC_SOCKET_PATH);
}

MessageSocketConnection *Client::socketConnection() const
{
    return m_connection;
}

void Client::onConnected()
{
    m_connectionRetryTimer->stop();
    Q_EMIT connected();
    ThreadUtils::runOnThread(ThreadUtils::mainThread(), [this]() {
        HandshakeRequestMessage handshakeRequest;
        if (const auto response = m_connection->sendMessageAndWaitForResponse<HandshakeResponseMessage>(handshakeRequest); !response->success()) {
            qCritical().noquote().nospace() << "Handshake failed: " << response->error();
            QCoreApplication::exit(-1);
            return;
        }

        BeginSessionRequestMessage beginSessionRequest;
        beginSessionRequest.setTty(m_currentTty);
        if (const auto response = m_connection->sendMessageAndWaitForResponse<BeginSessionResponseMessage>(beginSessionRequest)) {
            if (!response->success()) {
                qCritical().noquote().nospace() << "Daemon rejected request to begin session: " << response->error();
                QCoreApplication::exit(-1);
                return;
            }
        } else {
            qCritical() << "Daemon did not reply to session begin request";
            QCoreApplication::exit(-1);
            return;
        }

        LoadConfigRequestMessage configRequest;
        configRequest.setConfig(configProvider.currentConfig());
        m_connection->sendMessageAndWaitForResponse<LoadConfigResponseMessage>(configRequest);
    });
}

void Client::onDisconnected()
{
    m_connectionRetryTimer->start();
}

void Client::onErrorOccurred(QLocalSocket::LocalSocketError error)
{
    qCDebug(INPUTACTIONS_IPC).noquote().nospace() << "Failed to connect to server: " << error;
    m_connectionRetryTimer->start();
}

void Client::onConfigChanged(const QString &config)
{
    LoadConfigRequestMessage request;
    request.setConfig(config);
    m_connection->sendMessage(request);
}

void Client::connectToDaemon()
{
    m_connection->socket()->connectToServer(INPUTACTIONS_IPC_SOCKET_PATH);
}

}