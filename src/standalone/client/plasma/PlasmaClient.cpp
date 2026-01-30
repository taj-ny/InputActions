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

#include "PlasmaClient.h"
#include <QDBusInterface>

namespace InputActions
{

static const QString KWIN_SCRIPT_PATH = QString(INPUTACTIONS_DATA_DIR) + "/plasma/script.js";

PlasmaClient::~PlasmaClient()
{
    if (m_kwinScriptInterface) {
        m_kwinScriptInterface->call("stop");
    }
}

bool PlasmaClient::initialize()
{
    if (qEnvironmentVariable("XDG_CURRENT_DESKTOP") != "KDE") {
        return false;
    }

    QDBusInterface scripting("org.kde.KWin", "/Scripting", "org.kde.kwin.Scripting");
    scripting.call("unloadScript", "inputactions");
    const auto reply = scripting.call("loadScript", KWIN_SCRIPT_PATH, "inputactions");
    if (reply.arguments().size() == 0) {
        return false;
    }

    const auto scriptId = reply.arguments().at(0).toInt();
    m_kwinScriptInterface = std::make_unique<QDBusInterface>("org.kde.KWin", "/Scripting/Script" + QString::number(scriptId), "org.kde.kwin.Script");
    m_kwinScriptInterface->call("run");

    return true;
}

}