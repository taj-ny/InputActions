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

#include <libinputactions/Config.h>
#include <libinputactions/DBusInterface.h>

#include "interfaces/WaylandWindowProvider.h"
#include "protocols/WlrForeignToplevelManagementV1.h"

#include <QCoreApplication>

int main()
{
    static int argc = 0;
    QCoreApplication app(argc, nullptr);

    LibinputInputBackend backend;
    libinputactions::Config config(&backend);
    libinputactions::DBusInterface dbusInterface(&config);
    config.load(false);

    libinputactions::WindowProvider::setInstance(std::make_shared<WaylandWindowProvider>());

    auto *display = wl_display_connect(nullptr);
    auto *registry = wl_display_get_registry(display);
    WlrForeignToplevelManagementV1::instance()->initialize(registry);

    backend.initialize();
    while (true) {
        wl_display_roundtrip(display); // not sure if this is correct
        QCoreApplication::processEvents();
        backend.poll();
        usleep(1000);
    }
}