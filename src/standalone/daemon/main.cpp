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
#include "SessionManager.h"
#include "input/StandaloneInputBackend.h"
#include "interfaces/EvdevInputEmitter.h"
#include "interfaces/IPCNotificationManager.h"
#include "interfaces/IPCProcessRunner.h"
#include <QCoreApplication>
#include <QThread>
#include <csignal>
#include <libinputactions/InputActions.h>
#include <libinputactions/interfaces/ConfigProvider.h>
#include <libinputactions/interfaces/NotificationManager.h>

using namespace InputActions;

void handleSignal(int signal)
{
    if (signal == SIGINT) {
        QCoreApplication::quit();
    }
}

int main()
{
    static int argc = 0;
    QCoreApplication app(argc, nullptr);

    std::signal(SIGINT, handleSignal);

    ::InputActions::InputActions inputActions;
    g_inputBackend = std::make_unique<StandaloneInputBackend>();
    g_inputEmitter = std::make_shared<EvdevInputEmitter>();
    g_notificationManager = std::make_shared<IPCNotificationManager>();
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