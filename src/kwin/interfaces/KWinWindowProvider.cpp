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

#include "KWinWindowProvider.h"
#include "KWinWindow.h"
#include "effect/effecthandler.h"
#include "workspace.h"

std::unique_ptr<libinputactions::Window> KWinWindowProvider::activeWindow()
{
    if (auto *window = KWin::effects->activeWindow()) {
        return std::make_unique<KWinWindow>(window->window());
    }
    return {};
}

std::unique_ptr<libinputactions::Window> KWinWindowProvider::windowUnderPointer()
{
    if (auto *window = KWin::workspace()->windowUnderMouse(KWin::workspace()->activeOutput())) {
        return std::make_unique<KWinWindow>(window);
    }
    return {};
}