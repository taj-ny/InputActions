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

#include "Plugin.h"
#include "interfaces/HyprlandInputEmitter.h"
#include "interfaces/HyprlandPointer.h"
#include "interfaces/HyprlandWindowProvider.h"

#include <hyprland/src/managers/eventLoop/EventLoopManager.hpp>
#undef HANDLE

#include <QCoreApplication>

#include <chrono>

static auto s_qtEventLoopTickInterval = std::chrono::milliseconds(static_cast<uint32_t>(1));

Plugin::Plugin(void *handle)
    : m_handle(handle)
    , m_backend(new HyprlandInputBackend(this))
    , m_config(m_backend)
    , m_eventLoopTimer(makeShared<CEventLoopTimer>(s_qtEventLoopTickInterval, [this](SP<CEventLoopTimer> self, void* data) { eventLoopTick(); }, this))
{
    libinputactions::InputBackend::setInstance(std::unique_ptr<HyprlandInputBackend>(m_backend));
    libinputactions::InputEmitter::setInstance(std::make_shared<HyprlandInputEmitter>());
    libinputactions::PointerPositionGetter::setInstance(std::make_shared<HyprlandPointer>());
    libinputactions::WindowProvider::setInstance(std::make_unique<HyprlandWindowProvider>());

    g_pEventLoopManager->addTimer(m_eventLoopTimer);

    m_config.load();
}

void *Plugin::handle() const
{
    return m_handle;
}

void Plugin::eventLoopTick()
{
    QCoreApplication::processEvents();
    m_eventLoopTimer->updateTimeout(s_qtEventLoopTickInterval);
}