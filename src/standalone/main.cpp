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

#include "input/StandaloneInputBackend.h"
#include "interfaces/DBusInterfaceCollection.h"
#include "interfaces/StandaloneInputEmitter.h"
#include "protocols/VirtualKeyboardUnstableV1.h"
#include "protocols/WaylandProtocolManager.h"
#include "protocols/WlSeat.h"
#include "protocols/WlrForeignToplevelManagementV1.h"
#include "protocols/WlrVirtualPointerUnstableV1.h"
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEnvironmentVariables>
#include <libinputactions/InputActions.h>
#include <libinputactions/config/Config.h>
#include <print>

using namespace InputActions;

static const QDir GNOME_EXTENSION_DIR = QDir::homePath() + "/.local/share/gnome-shell/extensions/inputactions@inputactions.org";
static const auto GNOME_EXTENSION_EXTENSIONJS_PATH = GNOME_EXTENSION_DIR.path() + "/extension.js";
static const auto GNOME_EXTENSION_METADATAJSON_PATH = GNOME_EXTENSION_DIR.path() + "/metadata.json";
static const uint32_t GNOME_EXTENSION_VERSION = 1;

void installGnomeExtension()
{
    QFile extensionFile(GNOME_EXTENSION_EXTENSIONJS_PATH);
    QFile metadataFile(GNOME_EXTENSION_METADATAJSON_PATH);

    if (GNOME_EXTENSION_DIR.exists()) {
        if (metadataFile.exists()) {
            metadataFile.open(QIODeviceBase::ReadOnly);
            const auto jsonDocument = QJsonDocument::fromJson(metadataFile.readAll());
            const auto jsonObject = jsonDocument.object();
            if (jsonObject["version"].toDouble() == GNOME_EXTENSION_VERSION) {
                return;
            }
        }
    } else {
        GNOME_EXTENSION_DIR.mkpath(".");
    }

    extensionFile.remove();
    metadataFile.remove();
    QFile::copy(":/extensions/gnome/extension.js", GNOME_EXTENSION_EXTENSIONJS_PATH);
    QFile::copy(":/extensions/gnome/metadata.json", GNOME_EXTENSION_METADATAJSON_PATH);
}

int main()
{
    std::println(PROJECT_NAME " v" PROJECT_VERSION);
    std::println("Starting... ");

    static int argc = 0;
    QCoreApplication app(argc, nullptr);

    if (qEnvironmentVariable("XDG_CURRENT_DESKTOP").toLower().contains("gnome")) {
        installGnomeExtension();
    }

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

    ::InputActions::InputActions inputActions;
    g_inputBackend = std::make_unique<StandaloneInputBackend>();

    auto dbusInterfaceCollection = std::make_shared<DBusInterfaceCollection>();
    if (g_wlrForeignToplevelManagementV1->supported()) {
        g_windowProvider = std::make_shared<WlrForeignToplevelManagementV1WindowProvider>();
    } else {
        g_windowProvider = dbusInterfaceCollection;
    }
    g_pointerPositionGetter = dbusInterfaceCollection;
    g_inputEmitter = std::make_shared<StandaloneInputEmitter>();

    g_config->load(false);

    std::println("done");

    while (true) {
        g_inputBackend->poll();
        QCoreApplication::processEvents();
        wl_display_roundtrip(display);
        dynamic_cast<StandaloneInputBackend *>(g_inputBackend.get())->waitForEvents(5);
    }
}