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

#pragma once

#include "ClientDBusInterface.h"
#include <QLocalSocket>
#include <QObject>
#include <libinputactions/interfaces/implementations/FileConfigProvider.h>

namespace InputActions
{

class ClientDBusInterface;
class Message;
class MessageSocketConnection;

class Client : public QObject
{
    Q_OBJECT

public:
    Client();
    ~Client() override;

    Q_INVOKABLE void start();
    MessageSocketConnection *socketConnection() const;

    FileConfigProvider configProvider;

signals:
    void connected();
    void messageReceived(std::shared_ptr<Message> message);

private slots:
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QLocalSocket::LocalSocketError error);
    void onConfigChanged(const QString &config);

private:
    void connectToDaemon();

    MessageSocketConnection *m_connection;
    QTimer *m_connectionRetryTimer{};

    ClientDBusInterface m_dbusInterface;

    QString m_currentTty;
};

}