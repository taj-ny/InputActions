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

#include "WaylandProtocol.h"

#include <QLoggingCategory>

WaylandProtocol::WaylandProtocol(QString name)
    : m_name(std::move(name))
{
}

void WaylandProtocol::initialize(wl_registry *registry)
{
    static const wl_registry_listener listener{
        &WaylandProtocol::handleGlobal
    };
    wl_registry_add_listener(registry, &listener, this);
}

bool WaylandProtocol::supported() const
{
    return m_supported;
}

void WaylandProtocol::handleGlobal(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    auto *self = static_cast<WaylandProtocol *>(data);
    if (strcmp(interface, self->m_name.toStdString().c_str()) == 0) {
        self->m_supported = true;
        self->bind(registry, name);
    }
}