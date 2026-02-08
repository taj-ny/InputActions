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

#include <QDBusInterface>
#include <QTimer>
#include <libinputactions/dbus/DBusInterfaceBase.h>
#include <libinputactions/ipc/MessageHandler.h>
#include <libinputactions/ipc/messages.h>
#include <memory>

namespace InputActions
{

class FileConfigProvider;
class IPCEnvironmentInterfaces;
class Message;
class MessageSocketConnection;
class Server;
class VariableManager;

class Session
{
public:
    MessageSocketConnection *client() const { return m_client; }

private:
    bool m_hasClient{};
    QString m_config;
    MessageSocketConnection *m_client{};
    bool m_suspended{};

    std::shared_ptr<IPCEnvironmentInterfaces> m_ipcEnvironmentInterfaces;
    std::shared_ptr<VariableManager> m_variableManager;

    friend class SessionManager;
};

class SessionManager
    : public QObject
    , public MessageHandler
{
    Q_OBJECT

public:
    SessionManager(Server *server);
    ~SessionManager() override;

    Session &currentSession();
    Session *sessionForClient(MessageSocketConnection *client);

protected:
    void beginSessionRequestMessage(const std::shared_ptr<const BeginSessionRequestMessage> &message) override;
    void deviceListRequestMessage(const std::shared_ptr<const DeviceListRequestMessage> &message) override;
    void environmentStateMessage(const std::shared_ptr<const EnvironmentStateMessage> &message) override;
    void handshakeRequestMessage(const std::shared_ptr<const HandshakeRequestMessage> &message) override;
    void loadConfigRequestMessage(const std::shared_ptr<const LoadConfigRequestMessage> &message) override;
    void recordStrokeRequestMessage(const std::shared_ptr<const RecordStrokeRequestMessage> &message) override;
    void suspendRequestMessage(const std::shared_ptr<const SuspendRequestMessage> &message) override;
    void variableListRequestMessage(const std::shared_ptr<const VariableListRequestMessage> &message) override;

private slots:
    void onSessionChangeDetectionTimerTick();

private:
    void activateSession(Session &session, bool loadConfig = true);

    DBusInterfaceBase m_dbusInterfaceBase;
    QDBusInterface m_freedesktopLoginDbusInterface;

    std::unique_ptr<FileConfigProvider> m_etcConfigProvider;

    QTimer m_sessionChangeDetectionTimer;
    QString m_currentTty;
    Session *m_currentSession{};
    std::map<QString, Session> m_sessions;
};

inline std::shared_ptr<SessionManager> g_sessionManager;

}