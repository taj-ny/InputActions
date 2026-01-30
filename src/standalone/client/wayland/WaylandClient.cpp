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

#include "WaylandClient.h"
#include "WaylandProtocolManager.h"
#include "WlrForeignToplevelManagementV1.h"
#include <QObject>

namespace InputActions
{

WaylandClient::WaylandClient(Client *client)
    : m_client(client)
{
    connect(&m_displayDispatchTimer, &QTimer::timeout, this, &WaylandClient::onDisplayDispatchTimerTick);
    m_displayDispatchTimer.setInterval(100);
}

WaylandClient::~WaylandClient() = default;

bool WaylandClient::initialize()
{
    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        return false;
    }

    m_protocolManager = std::make_unique<WaylandProtocolManager>(wl_display_get_registry(m_display));
    m_protocolManager->addProtocol(std::make_unique<WlrForeignToplevelManagementV1>(m_client));
    m_displayDispatchTimer.start();
    return true;
}

void WaylandClient::onDisplayDispatchTimerTick()
{
    wl_display_roundtrip(m_display);
}

}