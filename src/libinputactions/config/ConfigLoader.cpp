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

#include "ConfigLoader.h"
#include "GlobalConfig.h"
#include "interfaces/ConfigProvider.h"
#include "yaml.h"
#include <QFile>
#include <QStandardPaths>
#include <libinputactions/actions/ActionExecutor.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/handlers/TouchscreenTriggerHandler.h>
#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/input/devices/InputDeviceRule.h>
#include <libinputactions/interfaces/NotificationManager.h>

namespace InputActions
{

/**
 * Used to detect and prevent infinite compositor crash loops when loading the configuration.
 */
const static QString CRASH_PREVENTION_FILE = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/inputactions_init";

struct Config
{
    bool allowExternalVariableAccess = true;
    bool autoReload = true;
    bool libevdevEnabled = true;
    bool sendNotificationOnError = true;

    std::unique_ptr<KeyboardTriggerHandler> keyboardTriggerHandler;
    std::unique_ptr<MouseTriggerHandler> mouseTriggerHandler;
    std::unique_ptr<PointerTriggerHandler> pointerTriggerHandler;
    std::function<std::unique_ptr<TouchpadTriggerHandler>(InputDevice *device)> touchpadTriggerHandlerFactory;
    std::function<std::unique_ptr<TouchscreenTriggerHandler>(InputDevice *device)> touchscreenTriggerHandlerFactory;

    std::vector<InputDeviceRule> deviceRules;
    std::set<KeyboardKey> emergencyCombination = {KEY_BACKSPACE, KEY_SPACE, KEY_ENTER};
};

void ConfigLoader::loadEmpty()
{
    activateConfig({}, false);
}

std::optional<QString> ConfigLoader::load(const ConfigLoadSettings &settings)
{
    std::optional<QString> error;

    if (!QFile::exists(CRASH_PREVENTION_FILE) || !settings.preventCrashLoops) {
        std::ignore = QFile(CRASH_PREVENTION_FILE).open(QIODevice::WriteOnly);

        try {
            qCDebug(INPUTACTIONS, "Reloading config");
            const auto rawConfig = settings.config.value_or(g_configProvider->currentConfig());

            auto config = createConfig(rawConfig);
            activateConfig(std::move(config), true);
        } catch (const std::exception &e) {
            error = QString("Failed to load configuration: %1").arg(QString::fromStdString(e.what()));
        }
    } else {
        error = "Configuration was not loaded automatically due to a crash.";
    }

    if (error) {
        qCCritical(INPUTACTIONS).noquote() << error.value();
        if (g_globalConfig->sendNotificationOnError() && !settings.manual) {
            g_notificationManager->sendNotification("Failed to load configuration", error.value());
        }
    }
    QFile::remove(CRASH_PREVENTION_FILE);
    return error;
}

Config ConfigLoader::createConfig(const QString &raw)
{
    const auto root = YAML::Load(raw.toStdString());

    Config config;
    YAML::loadMember(config.autoReload, root["autoreload"]);
    YAML::loadMember(config.allowExternalVariableAccess, root["external_variable_access"]);
    if (const auto &notificationsNode = root["notifications"]) {
        YAML::loadMember(config.sendNotificationOnError, notificationsNode["config_error"]);
    }
    YAML::loadMember(config.libevdevEnabled, root["__libevdev_enabled"]);
    YAML::loadMember(config.deviceRules, root);
    YAML::loadMember(config.emergencyCombination, root["emergency_combination"]);

    YAML::loadMember(config.keyboardTriggerHandler, root["keyboard"]);
    YAML::loadMember(config.mouseTriggerHandler, root["mouse"]);
    YAML::loadMember(config.pointerTriggerHandler, root["pointer"]);

    if (const auto &touchpadNode = root["touchpad"]) {
        config.touchpadTriggerHandlerFactory = [touchpadNode](auto *device) {
            return YAML::asTouchpadTriggerHandler(touchpadNode, device);
        };
        config.touchpadTriggerHandlerFactory(nullptr); // Make sure it doesn't throw
    }
    if (const auto &touchscreenNode = root["touchscreen"]) {
        config.touchscreenTriggerHandlerFactory = [touchscreenNode](auto *device) {
            return YAML::asTouchscreenTriggerHandler(touchscreenNode, device);
        };
        config.touchscreenTriggerHandlerFactory(nullptr);
    }

    return config;
}

void ConfigLoader::activateConfig(Config config, bool initialize)
{
    g_inputBackend->reset(); // Okay because required keys are not cleared
    g_actionExecutor->clearQueue();
    g_actionExecutor->waitForDone();

    g_globalConfig->setAllowExternalVariableAccess(config.allowExternalVariableAccess);
    g_globalConfig->setAutoReload(config.autoReload);
    g_globalConfig->setSendNotificationOnError(config.sendNotificationOnError);

    if (auto *libevdev = dynamic_cast<LibevdevComplementaryInputBackend *>(g_inputBackend.get())) {
        libevdev->setEnabled(config.libevdevEnabled);
    }

    g_inputBackend->setKeyboardTriggerHandler(std::move(config.keyboardTriggerHandler));
    g_inputBackend->setMouseTriggerHandler(std::move(config.mouseTriggerHandler));
    g_inputBackend->setPointerTriggerHandler(std::move(config.pointerTriggerHandler));
    g_inputBackend->setTouchpadTriggerHandlerFactory(config.touchpadTriggerHandlerFactory);
    g_inputBackend->setTouchscreenTriggerHandlerFactory(config.touchscreenTriggerHandlerFactory);
    g_inputBackend->setDeviceRules(config.deviceRules);
    g_inputBackend->setEmergencyCombination(config.emergencyCombination);

    if (initialize) {
        g_inputBackend->initialize();
    }
}

}