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

#include "input/LibinputInputBackend.h"

#include <libinputactions/config/Config.h>
#include <libinputactions/InputActions.h>

#include "interfaces/StandaloneInputEmitter.h"
#include "interfaces/StandaloneWindowProvider.h"
#include "protocols/VirtualKeyboardUnstableV1.h"
#include "protocols/WlrForeignToplevelManagementV1.h"
#include "protocols/WlrVirtualPointerUnstableV1.h"
#include "protocols/WaylandProtocolManager.h"
#include "protocols/WlSeat.h"

#include <QCoreApplication>

#include <print>

using namespace libinputactions;

int main()
{
    std::println(PROJECT_NAME " v" PROJECT_VERSION);
    std::println();

    static int argc = 0;
    QCoreApplication app(argc, nullptr);

    g_virtualKeyboardUnstableV1 = std::make_unique<VirtualKeyboardUnstableV1>();
    g_wlrForeignToplevelManagementV1 = std::make_unique<WlrForeignToplevelManagementV1>();
    g_wlrVirtualPointerUnstableV1 = std::make_unique<WlrVirtualPointerUnstableV1>();
    g_wlSeat = std::make_unique<WlSeat>();

    auto *display = wl_display_connect(nullptr);
    auto *registry = wl_display_get_registry(display);
    WaylandProtocolManager protocolManager(registry);
    protocolManager.addProtocol(g_virtualKeyboardUnstableV1.get());
    protocolManager.addProtocol(g_wlrForeignToplevelManagementV1.get());
    protocolManager.addProtocol(g_wlrVirtualPointerUnstableV1.get());
    protocolManager.addProtocol(g_wlSeat.get());
    wl_display_roundtrip(display);

    InputActions inputActions(std::make_unique<LibinputInputBackend>());
    g_inputEmitter = std::make_shared<StandaloneInputEmitter>();
    g_windowProvider = std::make_shared<StandaloneWindowProvider>();

    g_config->load(false);

    while (true) {
        wl_display_roundtrip(display); // not sure if this is correct
        QCoreApplication::processEvents();
        g_inputBackend->poll();
        usleep(1000);
    }
}