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

#include "WaylandProtocol.h"

class WaylandProtocolManager
{
public:
    WaylandProtocolManager(wl_registry *registry);

    void addProtocol(WaylandProtocol *protocol);

private:
    static void handleGlobal(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version);

    std::vector<WaylandProtocol *> m_protocols;
};