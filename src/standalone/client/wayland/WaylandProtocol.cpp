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

#include "WaylandProtocol.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(INPUTACTIONS_WAYLAND, "inputactions.wayland", QtWarningMsg);

namespace InputActions
{

WaylandProtocol::WaylandProtocol(QString interface)
    : m_interface(std::move(interface))
{
}

void WaylandProtocol::bind(wl_registry *registry, uint32_t name, uint32_t version)
{
    qCDebug(INPUTACTIONS_WAYLAND).noquote() << QString("Bound protocol %1").arg(m_interface);

    m_name = name;
    m_supported = true;
}

void WaylandProtocol::destroy()
{
    qCDebug(INPUTACTIONS_WAYLAND).noquote() << QString("Destroyed protocol %1").arg(m_interface);
}

}