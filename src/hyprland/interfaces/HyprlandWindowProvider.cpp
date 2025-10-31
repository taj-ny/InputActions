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

#include "HyprlandWindowProvider.h"
#include "HyprlandWindow.h"
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/managers/PointerManager.hpp>

using namespace InputActions;

std::shared_ptr<Window> HyprlandWindowProvider::activeWindow()
{
    if (auto *window = g_pCompositor->m_lastWindow.lock().get()) {
        return std::make_shared<HyprlandWindow>(window);
    }
    return {};
}

std::shared_ptr<Window> HyprlandWindowProvider::windowUnderPointer()
{
    if (auto *window = g_pCompositor->vectorToWindowUnified(g_pPointerManager->position(), 0).get()) {
        return std::make_shared<HyprlandWindow>(window);
    }
    return {};
}