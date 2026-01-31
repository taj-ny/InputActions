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

#include "HyprlandWindow.h"
#include <hyprland/src/desktop/view/Window.hpp>

namespace InputActions
{

HyprlandWindow::HyprlandWindow(Desktop::View::CWindow *window)
    : m_window(window)
{
}

std::optional<QString> HyprlandWindow::id()
{
    return QString::number((uintptr_t)m_window, 16);
}

std::optional<pid_t> HyprlandWindow::pid()
{
    return m_window->getPID();
}

std::optional<QRectF> HyprlandWindow::geometry()
{
    return QRectF(m_window->m_position.x, m_window->m_position.y, m_window->m_size.x, m_window->m_size.y);
}

std::optional<QString> HyprlandWindow::title()
{
    return QString::fromStdString(m_window->m_title);
}

std::optional<QString> HyprlandWindow::resourceClass()
{
    return QString::fromStdString(m_window->m_class);
}

std::optional<bool> HyprlandWindow::fullscreen()
{
    return m_window->isFullscreen();
}

}