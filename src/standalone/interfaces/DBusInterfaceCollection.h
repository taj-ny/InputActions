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

#pragma once

#include <QDBusConnection>
#include <QObject>
#include <libinputactions/interfaces/PointerPositionGetter.h>
#include <libinputactions/interfaces/Window.h>
#include <libinputactions/interfaces/WindowProvider.h>

class DBusWindow : public InputActions::Window
{
    std::optional<QString> id() override;
    std::optional<QRectF> geometry() override;
    std::optional<QString> title() override;
    std::optional<QString> resourceClass() override;
    std::optional<QString> resourceName() override;
    std::optional<bool> maximized() override;
    std::optional<bool> fullscreen() override;

private:
    std::optional<QString> m_id;
    std::optional<QRectF> m_geometry;
    std::optional<QString> m_title;
    std::optional<QString> m_resourceClass;
    std::optional<QString> m_resourceName;
    std::optional<bool> m_maximized;
    std::optional<bool> m_fullscreen;

    friend class DBusInterfaceCollection;
};

/**
 * Allows extensions and plugins to expose information through a DBus interface.
 */
class DBusInterfaceCollection
    : public QObject
    , public InputActions::PointerPositionGetter
    , public InputActions::WindowProvider
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.inputactions")

public:
    DBusInterfaceCollection();
    ~DBusInterfaceCollection() override;

    std::shared_ptr<InputActions::Window> activeWindow() override;
    std::shared_ptr<InputActions::Window> windowUnderPointer() override;

    std::optional<QPointF> globalPointerPosition() override;
    std::optional<QPointF> screenPointerPosition() override;

signals:
    /**
     * The extension must listen for this signal and send the requested keys by invoking the data method as soon as possible. Keys can be omitted if not
     * supported.
     *
     * @param keys The list of requested keys. If empty, all keys have been requested.
     */
    void dataRequested(QStringList keys);

public slots:
    /**
     * Called by extensions in order to inform InputActions of the current state of the environment. Extensions should call this as soon as the state changes.
     * The update rate of certain properties (e.g. pointer position) may be limited by the extension.
     *
     * A change of the active_window_id or window_under_pointer_id values signifies that the extension is sending information about a different window.
     *
     * @param data A JSON object, see the implementation for keys and what information they contain.
     */
    Q_NOREPLY void data(QString data);

private:
    QDBusConnection m_bus;

    std::shared_ptr<DBusWindow> m_activeWindow;
    std::shared_ptr<DBusWindow> m_windowUnderPointer;

    std::optional<QPointF> m_globalPointerPosition;
    std::optional<QPointF> m_screenPointerPosition;
};