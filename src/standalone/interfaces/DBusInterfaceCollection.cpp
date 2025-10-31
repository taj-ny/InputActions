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

#include "DBusInterfaceCollection.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

using namespace InputActions;

static const auto PATH = QStringLiteral("/Data");

DBusInterfaceCollection::DBusInterfaceCollection()
    : m_bus(QDBusConnection::sessionBus())
    , m_activeWindow(std::make_shared<DBusWindow>())
{
    m_bus.registerObject(PATH, this, QDBusConnection::ExportAllContents);
    Q_EMIT dataRequested({});
}

DBusInterfaceCollection::~DBusInterfaceCollection()
{
    m_bus.unregisterObject(PATH);
}

std::shared_ptr<Window> DBusInterfaceCollection::activeWindow()
{
    return m_activeWindow;
}

std::shared_ptr<InputActions::Window> DBusInterfaceCollection::windowUnderPointer()
{
    return m_windowUnderPointer;
}

void DBusInterfaceCollection::data(QString data)
{
    const auto json = QJsonDocument::fromJson(data.toUtf8());
    const auto object = json.object();

    static const auto readBool = [](auto &member, const QJsonValue &jsonValue) {
        if (jsonValue.isBool()) {
            member = jsonValue.toBool();
        }
    };
    static const auto readPoint = [](auto &member, const QJsonValue &jsonValue) {
        if (jsonValue.isArray()) {
            const auto array = jsonValue.toArray();
            if (array.size() == 2 && array[0].isDouble() && array[1].isDouble()) {
                member = {array[0].toDouble(), array[1].toDouble()};
            }
        }
    };
    static const auto readRect = [](auto &member, const QJsonValue &jsonValue) {
        if (jsonValue.isArray()) {
            const auto array = jsonValue.toArray();
            if (array.size() == 4 && array[0].isDouble() && array[1].isDouble() && array[2].isDouble() && array[3].isDouble()) {
                member = {array[0].toDouble(), array[1].toDouble(), array[2].toDouble(), array[3].toDouble()};
            }
        }
    };
    static const auto readString = [](auto &member, const QJsonValue &jsonValue) {
        if (jsonValue.isString()) {
            member = jsonValue.toString();
        }
    };

    const auto activeWindowId = object["active_window_id"];
    if (activeWindowId.isString() || activeWindowId.isDouble()) {
        m_activeWindow = std::make_shared<DBusWindow>();
        m_activeWindow->m_id = activeWindowId.toVariant().toString();
    } else if (activeWindowId.isNull()) {
        m_activeWindow = {};
    }
    readString(m_activeWindow->m_resourceClass, object["active_window_class"]);
    readBool(m_activeWindow->m_fullscreen, object["active_window_fullscreen"]);
    readBool(m_activeWindow->m_maximized, object["active_window_maximized"]);
    readString(m_activeWindow->m_resourceName, object["active_window_name"]);
    readString(m_activeWindow->m_title, object["active_window_title"]);

    const auto windowUnderPointerId = object["window_under_pointer_id"];
    if (windowUnderPointerId.isString() || windowUnderPointerId.isDouble()) {
        m_windowUnderPointer = std::make_shared<DBusWindow>();
        m_windowUnderPointer->m_id = windowUnderPointerId.toVariant().toString();
    } else if (windowUnderPointerId.isNull()) {
        m_windowUnderPointer = {};
    }
    readString(m_windowUnderPointer->m_resourceClass, object["window_under_pointer_class"]);
    readBool(m_windowUnderPointer->m_fullscreen, object["window_under_pointer_fullscreen"]);
    readRect(m_windowUnderPointer->m_geometry, object["window_under_pointer_geometry"]);
    readBool(m_windowUnderPointer->m_maximized, object["window_under_pointer_maximized"]);
    readString(m_windowUnderPointer->m_resourceName, object["window_under_pointer_name"]);
    readString(m_windowUnderPointer->m_title, object["window_under_pointer_title"]);

    readPoint(m_globalPointerPosition, object["pointer_position_global"]);
    readPoint(m_screenPointerPosition, object["pointer_position_screen_percentage"]);
}

std::optional<QString> DBusWindow::id()
{
    return m_id;
}

std::optional<QRectF> DBusWindow::geometry()
{
    return m_geometry;
}

std::optional<QString> DBusWindow::title()
{
    return m_title;
}

std::optional<QString> DBusWindow::resourceClass()
{
    return m_resourceClass;
}

std::optional<QString> DBusWindow::resourceName()
{
    return m_resourceName;
}

std::optional<bool> DBusWindow::maximized()
{
    return m_maximized;
}

std::optional<bool> DBusWindow::fullscreen()
{
    return m_fullscreen;
}

std::optional<QPointF> DBusInterfaceCollection::globalPointerPosition()
{
    return m_globalPointerPosition;
}

std::optional<QPointF> DBusInterfaceCollection::screenPointerPosition()
{
    return m_screenPointerPosition;
}