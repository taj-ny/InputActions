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

#include "Plugin.h"
#include "input/HyprlandInputBackend.h"
#include "interfaces/HyprlandOnScreenMessageManager.h"
#include "interfaces/HyprlandPointer.h"
#include "interfaces/HyprlandSessionLock.h"
#include "interfaces/HyprlandWindowProvider.h"
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/helpers/Monitor.hpp>
#include <hyprland/src/managers/eventLoop/EventLoopManager.hpp>
#include <libinputactions/config/Config.h>
#include <libinputactions/variables/VariableManager.h>
#undef HANDLE

#include <QCoreApplication>
#include <chrono>

using namespace InputActions;

static const auto TICK_INTERVAL = std::chrono::milliseconds(static_cast<uint32_t>(1));

Plugin::Plugin(void *handle)
{
    g_inputBackend = std::make_unique<HyprlandInputBackend>(handle);

    auto pointer = std::make_shared<HyprlandPointer>(handle);
    g_cursorShapeProvider = pointer;
    g_onScreenMessageManager = std::make_shared<HyprlandOnScreenMessageManager>();
    g_pointerPositionGetter = pointer;
    g_pointerPositionSetter = pointer;
    g_sessionLock = std::make_shared<HyprlandSessionLock>();
    g_windowProvider = std::make_shared<HyprlandWindowProvider>();

    m_eventLoopTimer = makeShared<CEventLoopTimer>(
        TICK_INTERVAL,
        [this](SP<CEventLoopTimer> self, void *data) {
            tick();
        },
        this);
    g_pEventLoopManager->addTimer(m_eventLoopTimer);

    setMissingImplementations();
    initialize();
    g_config->load(true);
}

void Plugin::registerGlobalVariables(VariableManager *variableManager, std::shared_ptr<PointerPositionGetter> pointerPositionGetter,
                                     std::shared_ptr<WindowProvider> windowProvider)
{
    InputActionsMain::registerGlobalVariables(variableManager, pointerPositionGetter, windowProvider);

    // This should be moved to libinputactions eventually
    variableManager->registerRemoteVariable<QString>("screen_name", [](auto &value) {
        if (const auto monitor = g_pCompositor->getMonitorFromCursor()) {
            value = QString::fromStdString(monitor->m_name);
        }
    });
}

void Plugin::tick()
{
    QCoreApplication::processEvents();
    m_eventLoopTimer->updateTimeout(TICK_INTERVAL);
}