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

#include "dbusinterface.h"
#include "effect/effecthandler.h"

#include <libinputactions/triggers/stroke.h>

static QString s_service = "org.inputactions";
static QString s_path = "/";

DBusInterface::DBusInterface()
{
    m_bus.registerService(s_service);
    m_bus.registerObject(s_path, this, QDBusConnection::ExportAllSlots);
}

DBusInterface::~DBusInterface()
{
    m_bus.unregisterService(s_service);
    m_bus.unregisterObject(s_path);
}

void DBusInterface::recordStroke(const QDBusMessage &message)
{
    KWin::effects->showOnScreenMessage("Input Actions is recording input. Perform a stroke gesture by moving your mouse or performing a touchpad swipe."
                                       " Recording will end after 250 ms of inactivity.");

    message.setDelayedReply(true);
    m_reply = message.createReply();

    auto backend = libinputactions::InputBackend::instance();
    connect(backend, &KWinInputBackend::strokeRecordingFinished, this, [this](const auto &stroke) {
        QByteArray bytes;
        const auto &points = stroke.points();
        for (size_t i = 0; i < points.size(); i++) {
            // All values range from -1 to 1
            bytes.push_back(static_cast<char>(points[i].x * 100));
            bytes.push_back(static_cast<char>(points[i].y * 100));
            bytes.push_back(static_cast<char>(points[i].t * 100));
            bytes.push_back(static_cast<char>(points[i].alpha * 100));
        }

        m_reply << QString("'%1'").arg(bytes.toBase64());
        m_bus.send(m_reply);

        KWin::effects->hideOnScreenMessage();
    }, Qt::SingleShotConnection);
    backend->recordStroke();
}