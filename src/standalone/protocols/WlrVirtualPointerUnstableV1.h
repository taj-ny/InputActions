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
#include "wlr-virtual-pointer-unstable-v1.h"
#include <libinputactions/interfaces/Window.h>

class WlrVirtualPointerUnstableV1Pointer
{
public:
    WlrVirtualPointerUnstableV1Pointer(zwlr_virtual_pointer_manager_v1 *manager);
    ~WlrVirtualPointerUnstableV1Pointer();

    void button(uint32_t button, bool state);
    void motion(QPointF delta);
    void frame();

private:
    zwlr_virtual_pointer_v1 *m_pointer{};
};

class WlrVirtualPointerUnstableV1 : public WaylandProtocol
{
public:
    WlrVirtualPointerUnstableV1();
    ~WlrVirtualPointerUnstableV1();

    std::unique_ptr<WlrVirtualPointerUnstableV1Pointer> createPointer();

protected:
    void bind(wl_registry *registry, uint32_t name, uint32_t version) override;

    zwlr_virtual_pointer_manager_v1 *m_manager{};
};

inline std::unique_ptr<WlrVirtualPointerUnstableV1> g_wlrVirtualPointerUnstableV1;