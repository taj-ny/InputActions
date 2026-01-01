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

#include "SessionManager.h"
#include "Server.h"
#include "interfaces/IPCEnvironmentInterfaces.h"
#include <QDBusArgument>
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/actions/ActionExecutor.h>
#include <libinputactions/config/Config.h>
#include <libinputactions/globals.h>
#include <libinputactions/input/StrokeRecorder.h>
#include <libinputactions/interfaces/implementations/FileConfigProvider.h>
#include <libinputactions/ipc/MessageSocketConnection.h>
#include <libinputactions/utils/SessionUtils.h>
#include <libinputactions/variables/VariableManager.h>
#include <pwd.h>
#include <sys/socket.h>
#include <utmp.h>

namespace InputActions
{

static const QString ERROR_SESSION_INACTIVE = "This client's session is inactive";

SessionManager::SessionManager(Server *server)
    : m_freedesktopLoginDbusInterface("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", QDBusConnection::systemBus())
{
    connect(server, &Server::messageReceived, this, [this](const auto &message) {
        handleMessage(message);
    });

    connect(&m_sessionChangeDetectionTimer, &QTimer::timeout, this, &SessionManager::onSessionChangeDetectionTimerTick);
    m_sessionChangeDetectionTimer.setInterval(1000);
    m_sessionChangeDetectionTimer.start();

    m_etcConfigProvider = std::make_unique<FileConfigProvider>();
    if (m_etcConfigProvider->currentPath() != INPUTACTIONS_ETC_CONFIG_PATH) {
        m_etcConfigProvider.reset();
    }
}

SessionManager::~SessionManager() = default;

Session &SessionManager::currentSession()
{
    return m_sessions[m_currentTty];
}

Session *SessionManager::sessionForClient(MessageSocketConnection *client)
{
    for (auto &[_, session] : m_sessions) {
        if (session.client() == client) {
            return &session;
        }
    }
    return {};
}

void SessionManager::beginSessionRequestMessage(const std::shared_ptr<const BeginSessionRequestMessage> &message)
{
    BeginSessionResponseMessage response;

    ucred cred;
    socklen_t len = sizeof(struct ucred);

    if (getsockopt(message->sender()->socket()->socketDescriptor(), SOL_SOCKET, SO_PEERCRED, &cred, &len) == -1) {
        response.setError("Authentication failed: could not get uid from connection");
        message->reply(response);
        return;
    }

    if (m_freedesktopLoginDbusInterface.isValid()) {
        const auto reply = m_freedesktopLoginDbusInterface.call("ListSessionsEx");
        if (reply.type() == QDBusMessage::MessageType::ErrorMessage) {
            response.setError(QString("Authentication failed: ListSessionsEx call failed: %1").arg(reply.errorMessage()));
            message->reply(response);
            return;
        }

        if (reply.arguments().count() == 0) {
            response.setError("Authentication failed: ListSessionEx returned no sessions");
            message->reply(response);
            return;
        }

        bool success{};
        const auto sessionData = reply.arguments().at(0).value<QDBusArgument>();
        sessionData.beginArray();
        while (!sessionData.atEnd()) {
            bool b;
            QDBusObjectPath o;
            QString s;
            quint64 t;
            uint32_t u;

            uint32_t uid;
            QString tty;

            sessionData.beginStructure();
            sessionData >> s >> uid >> s >> s >> u >> s >> tty >> b >> t >> o;
            sessionData.endStructure();

            if (cred.uid == uid && message->tty() == tty) {
                success = true;
                break;
            }
        }
        sessionData.endArray();

        if (!success) {
            response.setError("Permission denied: cannot begin session for another user");
            message->reply(response);
            return;
        }
    } else {
        QString ttyUser;
        setutent();
        utmp *entry;
        while ((entry = getutent()) != nullptr) {
            if (entry->ut_type == USER_PROCESS && message->tty() == entry->ut_line) {
                ttyUser = QString::fromLatin1(entry->ut_user, sizeof(entry->ut_user));
                break;
            }
        }
        endutent();

        if (ttyUser.isEmpty()) {
            response.setError("Authentication failed: could not get username of tty owner");
            message->reply(response);
            return;
        }

        passwd *pwd = getpwnam(ttyUser.toStdString().c_str());
        if (!pwd) {
            response.setError("Authentication failed: could not get pid from username");
            message->reply(response);
            return;
        }

        if (cred.uid != pwd->pw_uid) {
            response.setError("Permission denied: cannot begin session for another user");
            message->reply(response);
            return;
        }
    }

    auto &session = m_sessions[message->tty()];
    if (session.m_client) {
        response.setError("This TTY already has an initialized session");
    } else {
        session.m_client = message->sender();
        session.m_ipcEnvironmentInterfaces = std::make_shared<IPCEnvironmentInterfaces>();
        session.m_variableManager = std::make_shared<VariableManager>();
        g_inputActions->registerGlobalVariables(session.m_variableManager.get(), session.m_ipcEnvironmentInterfaces, session.m_ipcEnvironmentInterfaces);

        if (SessionUtils::currentTty() == message->tty()) {
            activateSession(session, false);
        }
    }
    message->reply(response);

    connect(message->sender()->socket(), &QLocalSocket::disconnected, this, [this, socket = message->sender()]() {
        if (auto *session = sessionForClient(socket)) {
            session->m_client = {};
            session->m_config = {};

            if (session == &currentSession()) {
                qCDebug(INPUTACTIONS) << "Client disconnected, suspending current session";
                activateSession(*session);
            }
        }
    });
}

void SessionManager::environmentStateMessage(const std::shared_ptr<const EnvironmentStateMessage> &message)
{
    if (auto *session = sessionForClient(message->sender())) {
        session->m_ipcEnvironmentInterfaces->updateEnvironmentState(message->stateJson());
    }
}

void SessionManager::handshakeRequestMessage(const std::shared_ptr<const HandshakeRequestMessage> &message)
{
    HandshakeResponseMessage response;
    if (message->protocolVersion() != INPUTACTIONS_IPC_PROTOCOL_VERSION) {
        response.setError(QString("Protocol version mismatch (daemon: %1, client: %2)")
                              .arg(QString::number(INPUTACTIONS_IPC_PROTOCOL_VERSION), QString::number(message->protocolVersion())));
    }
    message->reply(response);
}

void SessionManager::loadConfigRequestMessage(const std::shared_ptr<const LoadConfigRequestMessage> &message)
{
    LoadConfigResponseMessage response;
    if (auto *session = sessionForClient(message->sender())) {
        session->m_suspended = false;
        auto config = message->config();
        if (m_etcConfigProvider) {
            config = m_etcConfigProvider->currentConfig();
            qCDebug(INPUTACTIONS).noquote().nospace() << INPUTACTIONS_ETC_CONFIG_PATH << " exists, overriding local config";
        }

        session->m_config = config;
        if (&currentSession() == session) {
            if (const auto error = g_config->load(config)) {
                response.setError(error.value());
            }
        }
    }

    message->reply(response);
}

void SessionManager::recordStrokeRequestMessage(const std::shared_ptr<const RecordStrokeRequestMessage> &message)
{
    if (const auto *session = sessionForClient(message->sender())) {
        if (&currentSession() != session) {
            RecordStrokeResponseMessage response;
            response.setError(ERROR_SESSION_INACTIVE);
            message->reply(response);
            return;
        }

        g_strokeRecorder->recordStroke([this, message](const auto &stroke) {
            RecordStrokeResponseMessage response;
            response.setStroke(m_dbusInterfaceBase.strokeToBase64(stroke));
            message->reply(response);
        });
    }
}

void SessionManager::suspendRequestMessage(const std::shared_ptr<const SuspendRequestMessage> &message)
{
    if (auto *session = sessionForClient(message->sender())) {
        session->m_suspended = true;

        if (&currentSession() == session) {
            activateSession(*session, false);
        }

        message->reply();
    }
}

void SessionManager::variableListRequestMessage(const std::shared_ptr<const VariableListRequestMessage> &message)
{
    if (auto *session = sessionForClient(message->sender())) {
        VariableListResponseMessage response;
        response.setVariables(m_dbusInterfaceBase.variableList(session->m_variableManager.get(), message->filter()));
        message->reply(response);
    }
}

void SessionManager::activateSession(const Session &session, bool loadConfig)
{
    g_inputActions->suspend();
    g_actionExecutor->clearQueue();
    g_actionExecutor->waitForDone();

    if (session.m_suspended) {
        qCDebug(INPUTACTIONS) << "Session is suspended";
        return;
    }
    if (!session.m_client) {
        qCDebug(INPUTACTIONS) << "No client/config for current session, suspending";
        return;
    }

    if (loadConfig && g_config->load(session.m_config)) {
        g_config->load(QString(""));
    }

    g_pointerPositionGetter = session.m_ipcEnvironmentInterfaces;
    g_variableManager = session.m_variableManager;
    g_windowProvider = session.m_ipcEnvironmentInterfaces;
}

void SessionManager::onSessionChangeDetectionTimerTick()
{
    const auto tty = SessionUtils::currentTty();
    if (m_currentTty != tty) {
        qCDebug(INPUTACTIONS).noquote().nospace() << "TTY changed to " << tty;
        activateSession(m_sessions[tty]);
        m_currentTty = tty;
    }
}

}