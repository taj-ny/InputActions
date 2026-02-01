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

#include "DBusInterfaceBase.h"
#include <QRegularExpression>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/triggers/StrokeTrigger.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

QString DBusInterfaceBase::deviceList()
{
    QStringList result;
    for (const auto *device : g_inputBackend->devices()) {
        result.push_back(device->toString());
    }
    result.sort();
    return result.join("\n\n");
}

QString DBusInterfaceBase::strokeToBase64(const Stroke &stroke)
{
    QByteArray bytes;
    const auto &points = stroke.points();
    for (size_t i = 0; i < points.size(); i++) {
        // All values range from -1 to 1
        bytes.push_back(static_cast<char>(points[i].x * 100));
        bytes.push_back(static_cast<char>(points[i].y * 100));
        bytes.push_back(static_cast<char>(points[i].t * 100));
        bytes.push_back(static_cast<char>(points[i].alpha * 100));
    }

    return QString("'%1'").arg(bytes.toBase64());
}

QString DBusInterfaceBase::variableList(const VariableManager *variableManager, const QString &filter)
{
    QStringList result;
    const QRegularExpression filterRegex(filter);
    for (const auto &[name, variable] : variableManager->variables()) {
        if (variable->hidden() || !filterRegex.match(name).hasMatch()) {
            continue;
        }
        result.push_back(QString("%1: %2").arg(name, variable->operations()->toString()));
    }
    return result.join('\n');
}

}