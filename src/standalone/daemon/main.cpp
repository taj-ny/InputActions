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

#include "Server.h"
#include "SessionManager.h"
#include "input/StandaloneInputBackend.h"
#include "interfaces/IPCNotificationManager.h"
#include "interfaces/IPCPlasmaGlobalShortcutInvoker.h"
#include "interfaces/IPCProcessRunner.h"
#include <QCoreApplication>
#include <QDir>
#include <QThread>
#include <csignal>
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/globals.h>
#include <libinputactions/interfaces/ConfigProvider.h>
#include <libinputactions/interfaces/NotificationManager.h>
#include <sys/file.h>
#include <sys/stat.h>

using namespace InputActions;

static const QDir VAR_RUN_INPUTACTIONS_DIR("/var/run/inputactions");
static const QString LOCK_FILE_PATH = VAR_RUN_INPUTACTIONS_DIR.path() +
#ifdef DEBUG
    "/lock-debug";
#else
    "/lock";
#endif

void handleSignal(int signal)
{
    if (signal == SIGINT) {
        QCoreApplication::quit();
    }
}

int main()
{
    if (geteuid()) {
        qCritical() << "The daemon must be run as root.";
        return -1;
    }

    static int argc = 0;
    QCoreApplication app(argc, nullptr);

    std::signal(SIGINT, handleSignal);

    const int minPriority = sched_get_priority_min(SCHED_RR);
    sched_param sp;
    sp.sched_priority = minPriority;
    if (pthread_setschedparam(pthread_self(), SCHED_RR | SCHED_RESET_ON_FORK, &sp) != 0) {
        qWarning(INPUTACTIONS, "Failed to gain real time thread priority: %s", strerror(errno));
    }

    if (!VAR_RUN_INPUTACTIONS_DIR.exists()) {
        VAR_RUN_INPUTACTIONS_DIR.mkpath(".");
        chmod(VAR_RUN_INPUTACTIONS_DIR.path().toStdString().c_str(), 0755);
    }

    const auto fd = open(LOCK_FILE_PATH.toStdString().c_str(), O_RDWR | O_CREAT, 0600);
    if (flock(fd, LOCK_EX | LOCK_NB) < 0) {
        if (errno == EWOULDBLOCK) {
            qCritical() << "A daemon instance is already running.";
            return -1;
        }
    }

    InputActionsMain inputActions;
    g_inputBackend = std::make_unique<StandaloneInputBackend>();
    g_notificationManager = std::make_shared<IPCNotificationManager>();
    g_plasmaGlobalShortcutInvoker = std::make_shared<IPCPlasmaGlobalShortcutInvoker>();
    g_processRunner = std::make_shared<IPCProcessRunner>();

    auto *serverThread = new QThread;
    auto *server = new Server;
    server->moveToThread(serverThread);

    g_sessionManager = std::make_shared<SessionManager>(server);
    g_configProvider = std::make_shared<ConfigProvider>(); // Config is managed by SessionManager

    inputActions.setMissingImplementations();
    inputActions.initialize();

    QObject::connect(serverThread, &QThread::started, [server]() {
        QMetaObject::invokeMethod(server, "start");
    });
    serverThread->start();

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [server]() {
        server->deleteLater();
    });
    return app.exec();
}