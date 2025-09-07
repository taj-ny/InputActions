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

#include <QDBusConnection>
#include <QDBusMessage>
#include <QObject>

namespace libinputactions
{

class DBusInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.inputactions")

public:
    /**
     * Registers the interface.
     */
    DBusInterface();

    /**
     * Unregisters the interface.
     */
    ~DBusInterface() override;

public slots:
    Q_NOREPLY void recordStroke(const QDBusMessage &message);
    QString reloadConfig();
    QString variables(QString filter = "");

private:
    QDBusConnection m_bus;
    QDBusMessage m_reply;
};

}