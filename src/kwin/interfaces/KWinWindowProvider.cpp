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
#include <libinputactions/input/InputDevice.h>
#include <libinputactions/input/backends/InputBackend.h>
#include <window.h>

namespace InputActions
{

std::shared_ptr<Window> KWinWindowProvider::activeWindow()
{
    if (auto *window = KWin::effects->activeWindow()) {
        return std::make_shared<KWinWindow>(window->window());
    }
    return {};
}

std::shared_ptr<Window> KWinWindowProvider::windowUnderFingers()
{
    const auto *device = g_inputBackend->currentTouchscreen();
    if (!device) {
        return {};
    }

    QPointF center;
    const auto validTouchPoints = device->validTouchPoints();
    for (const auto &touchPoint : device->validTouchPoints()) {
        center += touchPoint->unalteredPosition / validTouchPoints.size();
    }

    if (center.isNull()) {
        return {};
    }

    const auto windows = KWin::workspace()->stackingOrder();
    for (auto it = windows.rbegin(); it != windows.rend(); ++it) {
        auto *window = *it;
        if (window->frameGeometry().contains(center)) {
            return std::make_shared<KWinWindow>(window);
        }
    }
    return {};
}

std::shared_ptr<Window> KWinWindowProvider::windowUnderPointer()
{
    if (auto *window = KWin::workspace()->windowUnderMouse(KWin::workspace()->activeOutput())) {
        return std::make_shared<KWinWindow>(window);
    }
    return {};
}

}