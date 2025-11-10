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

#include "WaylandProtocolManager.h"
#include "WaylandProtocol.h"

namespace InputActions
{

WaylandProtocolManager::WaylandProtocolManager(wl_registry *registry)
{
    static const wl_registry_listener listener(&WaylandProtocolManager::handleGlobal);
    wl_registry_add_listener(registry, &listener, this);
}

WaylandProtocolManager::~WaylandProtocolManager() = default;

void WaylandProtocolManager::addProtocol(std::unique_ptr<WaylandProtocol> protocol)
{
    m_protocols.push_back(std::move(protocol));
}

void WaylandProtocolManager::handleGlobal(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    auto *self = static_cast<WaylandProtocolManager *>(data);
    for (auto &protocol : self->m_protocols) {
        if (strcmp(interface, protocol->name().toStdString().c_str()) == 0) {
            protocol->bind(registry, name, version);
            return;
        }
    }
}

}