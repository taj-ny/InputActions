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

using namespace libinputactions;

static auto s_qtEventLoopTickInterval = std::chrono::milliseconds(static_cast<uint32_t>(1));

Plugin::Plugin(void *handle)
    : m_handle(handle)
    , m_backend(std::make_shared<HyprlandInputBackend>(this))
    , m_config(m_backend.get())
    , m_dbusInterface(&m_config)
    , m_eventLoopTimer(makeShared<CEventLoopTimer>(s_qtEventLoopTickInterval, [this](SP<CEventLoopTimer> self, void* data) { eventLoopTick(); }, this))
{
    InputBackend::setInstance(m_backend);
    InputEmitter::setInstance(std::make_shared<HyprlandInputEmitter>());
    PointerPositionGetter::setInstance(std::make_shared<HyprlandPointer>());
    WindowProvider::setInstance(std::make_unique<HyprlandWindowProvider>());

    g_pEventLoopManager->addTimer(m_eventLoopTimer);

    m_config.load();
}

Plugin::~Plugin()
{
    InputBackend::setInstance(nullptr);
    InputEmitter::setInstance(nullptr);
    PointerPositionGetter::setInstance(nullptr);
    WindowProvider::setInstance(nullptr);
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