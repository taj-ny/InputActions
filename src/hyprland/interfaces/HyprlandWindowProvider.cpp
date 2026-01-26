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
#include "input/HyprlandInputDevice.h"
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/state/FocusState.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <hyprland/src/managers/PointerManager.hpp>
#undef HANDLE
#include <libinputactions/input/InputDevice.h>
#include <libinputactions/input/backends/InputBackend.h>

namespace InputActions
{

std::shared_ptr<Window> HyprlandWindowProvider::activeWindow()
{
    if (auto *window = Desktop::focusState()->window().get()) {
        return std::make_shared<HyprlandWindow>(window);
    }
    return {};
}

std::shared_ptr<Window> HyprlandWindowProvider::windowUnderFingers()
{
    const auto *device = g_inputBackend->currentTouchscreen();
    if (!device) {
        return {};
    }
    const auto *hyprlandDevice = dynamic_cast<const ITouch *>(dynamic_cast<const HyprlandInputDevice *>(device)->hyprlandDevice());

    const auto validTouchPoints = device->validTouchPoints();
    if (validTouchPoints.empty()) {
        return {};
    }

    QPointF center;
    for (const auto &touchPoint : validTouchPoints) {
        center += touchPoint->rawPosition;
    }
    center /= validTouchPoints.size();

    auto monitor = g_pCompositor->getMonitorFromName(!hyprlandDevice->m_boundOutput.empty() ? hyprlandDevice->m_boundOutput : "");
    if (!monitor) {
        monitor = Desktop::focusState()->monitor();
    }

    const Vector2D position(monitor->m_position.x + center.x() * monitor->m_size.x, monitor->m_position.y + center.y() * monitor->m_size.y);
    if (auto *window = g_pCompositor->vectorToWindowUnified(position, 0).get()) {
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

}