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

#include "kwinwindow.h"

#include "effect/effecthandler.h"
#include "core/output.h"
#include "workspace.h"

KWinWindow::KWinWindow(KWin::Window *window)
    : m_window(window)
{
}

std::optional<QRectF> KWinWindow::geometry() const
{
    return m_window->frameGeometry();
}

std::optional<QString> KWinWindow::title() const
{
    return m_window->caption();
}

std::optional<QString> KWinWindow::resourceClass() const
{
    return m_window->resourceClass();
}

std::optional<QString> KWinWindow::resourceName() const
{
    return m_window->resourceName();
}

std::optional<bool> KWinWindow::maximized() const
{
    return m_window->maximizeMode() == KWin::MaximizeMode::MaximizeFull;
}

std::optional<bool> KWinWindow::fullscreen() const
{
    return m_window->isFullScreen();
}

std::shared_ptr<libinputactions::Window> KWinWindowProvider::active() const
{
    auto window = KWin::effects->activeWindow();
    if (!window) {
        return {};
    }
    return std::make_shared<KWinWindow>(window->window());
}

std::shared_ptr<libinputactions::Window> KWinWindowProvider::underPointer() const
{
    auto window = KWin::workspace()->windowUnderMouse(KWin::workspace()->activeOutput());
    if (!window) {
        return {};
    }
    return std::make_shared<KWinWindow>(window);
}