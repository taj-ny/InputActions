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

#include "KWinWindow.hpp"

#include "core/output.h"
#include "effect/effecthandler.h"
#include "window.h"

KWinWindow::KWinWindow(KWin::Window *window)
    : m_window(window)
{
}

std::optional<QString> KWinWindow::id()
{
    return m_window->internalId().toString();
}

std::optional<QRectF> KWinWindow::geometry()
{
    return m_window->frameGeometry();
}

std::optional<QString> KWinWindow::title()
{
    return m_window->caption();
}

std::optional<QString> KWinWindow::resourceClass()
{
    return m_window->resourceClass();
}

std::optional<QString> KWinWindow::resourceName()
{
    return m_window->resourceName();
}

std::optional<bool> KWinWindow::maximized()
{
    return m_window->maximizeMode() == KWin::MaximizeMode::MaximizeFull;
}

std::optional<bool> KWinWindow::fullscreen()
{
    return m_window->isFullScreen();
}