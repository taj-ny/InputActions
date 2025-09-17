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

#include "NotificationManager.h"
#include <QDBusInterface>
#include <QThreadPool>

namespace libinputactions
{

void NotificationManager::sendNotification(const QString &title, const QString &content)
{
    // Run in another thread because QDBusInterface's constructor can freeze the compositor if a notification is sent as soon as the plugin loads. Good enough
    // for now.
    QThreadPool::globalInstance()->start([title = std::move(title), content = std::move(content)] {
        QDBusInterface notificationsInterface("org.freedesktop.Notifications",
                                              "/org/freedesktop/Notifications",
                                              "org.freedesktop.Notifications",
                                              QDBusConnection::sessionBus());
        if (notificationsInterface.isValid()) {
            notificationsInterface.asyncCall("Notify", "InputActions", 0U, "", title, content, QStringList(), QVariantMap(), 5000);
        }
    });
}

}