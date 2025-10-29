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

#include "WlSeat.h"

WlSeat::WlSeat()
    : WaylandProtocol(wl_seat_interface.name)
{
}

WlSeat::~WlSeat()
{
    if (m_seat) {
        wl_seat_destroy(m_seat);
    }
}

wl_seat *WlSeat::seat()
{
    return m_seat;
}

void WlSeat::bind(wl_registry *registry, uint32_t name, uint32_t version)
{
    m_seat = static_cast<wl_seat *>(wl_registry_bind(registry, name, &wl_seat_interface, version));
}