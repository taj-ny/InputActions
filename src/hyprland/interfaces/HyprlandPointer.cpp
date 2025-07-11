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

#include "HyprlandPointer.h"
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <hyprland/src/managers/PointerManager.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/plugins/HookSystem.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#undef HANDLE

#include <QRectF>

using namespace libinputactions;

typedef void (*setCursorFromName)(void *thisPtr, const std::string &name);

HyprlandPointer::HyprlandPointer(void *handle)
    : m_setCursorFromNameHook(handle, "setCursorFromName", (void *)&setCursorFromNameHook)
{
}

std::optional<CursorShape> HyprlandPointer::cursorShape()
{
    if (CURSOR_SHAPES.contains(m_currentCursorShape)) {
        return CURSOR_SHAPES.at(m_currentCursorShape);
    }
    return {};
}

std::optional<QPointF> HyprlandPointer::globalPointerPosition()
{
    const auto position = g_pPointerManager->position();
    return QPointF(position.x, position.y);
}

std::optional<QPointF> HyprlandPointer::screenPointerPosition()
{
    const auto monitor = g_pCompositor->getMonitorFromCursor();
    const QRectF geometry(monitor->m_position.x, monitor->m_position.y, monitor->m_size.x, monitor->m_size.y);
    const auto translatedPosition = globalPointerPosition().value() - geometry.topLeft();
    return QPointF(translatedPosition.x() / geometry.width(), translatedPosition.y() / geometry.height());
}

void HyprlandPointer::setCursorFromNameHook(void *thisPtr, const std::string &name)
{
    auto *self = dynamic_cast<HyprlandPointer *>(g_cursorShapeProvider.get());
    (*(setCursorFromName)self->m_setCursorFromNameHook->m_original)(thisPtr, name);
    self->m_currentCursorShape = QString::fromStdString(name).replace('-', '_');
}