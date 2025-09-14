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

#include "WlrVirtualPointerUnstableV1.h"

WlrVirtualPointerUnstableV1::WlrVirtualPointerUnstableV1()
    : WaylandProtocol(zwlr_virtual_pointer_manager_v1_interface.name)
{
}

WlrVirtualPointerUnstableV1::~WlrVirtualPointerUnstableV1()
{
    if (m_manager) {
        zwlr_virtual_pointer_manager_v1_destroy(m_manager);
    }
}

void WlrVirtualPointerUnstableV1::bind(wl_registry *registry, uint32_t name, uint32_t version)
{
    WaylandProtocol::bind(registry, name, version);
    m_manager = static_cast<zwlr_virtual_pointer_manager_v1 *>(wl_registry_bind(registry, name, &zwlr_virtual_pointer_manager_v1_interface, version));
}

std::unique_ptr<WlrVirtualPointerUnstableV1Pointer> WlrVirtualPointerUnstableV1::createPointer()
{
    return std::make_unique<WlrVirtualPointerUnstableV1Pointer>(m_manager);
}

WlrVirtualPointerUnstableV1Pointer::WlrVirtualPointerUnstableV1Pointer(zwlr_virtual_pointer_manager_v1 *manager)
{
    m_pointer = zwlr_virtual_pointer_manager_v1_create_virtual_pointer(manager, nullptr);
}

WlrVirtualPointerUnstableV1Pointer::~WlrVirtualPointerUnstableV1Pointer()
{
    zwlr_virtual_pointer_v1_destroy(m_pointer);
}

void WlrVirtualPointerUnstableV1Pointer::button(uint32_t button, bool state)
{
    zwlr_virtual_pointer_v1_button(m_pointer, 0, button, state ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED);
}

void WlrVirtualPointerUnstableV1Pointer::motion(QPointF delta)
{
    zwlr_virtual_pointer_v1_motion(m_pointer, 0, wl_fixed_from_double(delta.x()), wl_fixed_from_double(delta.y()));
}

void WlrVirtualPointerUnstableV1Pointer::frame()
{
    zwlr_virtual_pointer_v1_frame(m_pointer);
}