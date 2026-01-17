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
#include "ConfigIssueManager.h"
#include "GlobalConfig.h"
#include "Node.h"
#include "interfaces/ConfigProvider.h"
#include "parsers/core.h"
#include "parsers/utils.h"
#include <QFile>
#include <QStandardPaths>
#include <libinputactions/actions/ActionExecutor.h>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/handlers/MouseTriggerHandler.h>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/handlers/TouchscreenTriggerHandler.h>
#include <libinputactions/input/InputDeviceRule.h>
#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/interfaces/InputEmitter.h>
#include <libinputactions/interfaces/NotificationManager.h>
#include <yaml-cpp/yaml.h>

namespace InputActions
{

/**
 * Used to detect and prevent infinite compositor crash loops when loading the configuration.
 */
const static QString CRASH_PREVENTION_FILE = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/inputactions_init";

struct Config
{
    bool autoReload = true;
    bool libevdevEnabled = true;
    bool sendNotificationOnError = true;

    std::unique_ptr<KeyboardTriggerHandler> keyboardTriggerHandler;
    std::unique_ptr<MouseTriggerHandler> mouseTriggerHandler;
    std::unique_ptr<PointerTriggerHandler> pointerTriggerHandler;
    std::function<std::unique_ptr<TouchpadTriggerHandler>(InputDevice *device)> touchpadTriggerHandlerFactory;
    std::function<std::unique_ptr<TouchscreenTriggerHandler>(InputDevice *device)> touchscreenTriggerHandlerFactory;

    std::vector<InputDeviceRule> deviceRules;
};

void ConfigLoader::loadEmpty()
{
    activateConfig({}, false);
}

bool ConfigLoader::load(ConfigLoadSettings settings)
{
    if (!QFile::exists(CRASH_PREVENTION_FILE) || !settings.preventCrashLoops) {
        std::ignore = QFile(CRASH_PREVENTION_FILE).open(QIODevice::WriteOnly);

        try {
            qCDebug(INPUTACTIONS, "Reloading config");
            const auto rawConfig = settings.config.value_or(g_configProvider->currentConfig());

            g_configIssueManager = std::make_shared<ConfigIssueManager>(rawConfig);
            auto config = createConfig(rawConfig);
            activateConfig(std::move(config), true);
        } catch (const YAML::Exception &e) {
            g_configIssueManager->addIssue(e.mark.line, e.mark.column, ConfigIssueSeverity::Error, QString("Syntax error: %1.").arg(e.what()));
        } catch (const ConfigParserException &e) {
            g_configIssueManager->addIssue(e.line(), e.column(), ConfigIssueSeverity::Error, e.what());
        }
    } else {
        g_configIssueManager->addIssue(-1, -1, ConfigIssueSeverity::Error, "Configuration was not loaded automatically due to a crash.");
    }

    const auto issues = g_configIssueManager->issues();
    auto error = std::ranges::find_if(issues, [](const auto &issue) {
        return issue.severity() == ConfigIssueSeverity::Error;
    });
    if (error != issues.end()) {
        if (g_globalConfig->sendNotificationOnError() && settings.sendNotificationOnError) {
            QString pos = "";
            if (error->line() != -1) {
                pos = QString("%1:%2: ").arg(QString::number(error->line() + 1), QString::number(error->column() + 1));
            }

            g_notificationManager->sendNotification("Failed to load configuration", pos + error->message());
        }
    }
    QFile::remove(CRASH_PREVENTION_FILE);
    return error == issues.end();
}

Config ConfigLoader::createConfig(const QString &raw)
{
    const Node root = YAML::Load(raw.toStdString());
    if (!root.isMap()) {
        throw ConfigParserException(&root, "Expected a map.");
    }

    Config config;
    loadMember(config.autoReload, root.at("autoreload").get());
    if (const auto notificationsNode = root.mapAt("notifications")) {
        loadMember(config.sendNotificationOnError, notificationsNode->at("config_error").get());
    }
    loadMember(config.libevdevEnabled, root.at("__libevdev_enabled").get());
    loadMember(config.deviceRules, &root);

    loadMember(config.keyboardTriggerHandler, root.mapAt("keyboard").get());
    loadMember(config.mouseTriggerHandler, root.mapAt("mouse").get());
    loadMember(config.pointerTriggerHandler, root.mapAt("pointer").get());

    if (const auto touchpadNode = root.mapAt("touchpad")) {
        touchpadNode->disableMapAccessCheck("devices");

        config.touchpadTriggerHandlerFactory = [touchpadNode](auto *device) {
            return parseTouchpadTriggerHandler(touchpadNode.get(), device);
        };
        config.touchpadTriggerHandlerFactory(nullptr); // Make sure it doesn't throw
    }
    if (const auto touchscreenNode = root.mapAt("touchscreen")) {
        config.touchscreenTriggerHandlerFactory = [touchscreenNode](auto *device) {
            return parseTouchscreenTriggerHandler(touchscreenNode.get(), device);
        };
        config.touchscreenTriggerHandlerFactory(nullptr);
    }

    return config;
}

void ConfigLoader::activateConfig(Config config, bool initialize)
{
    g_inputBackend->reset();
    g_inputEmitter->reset(); // Okay because required keys are not cleared
    g_actionExecutor->clearQueue();
    g_actionExecutor->waitForDone();

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

    if (initialize) {
        g_inputEmitter->initialize();
        g_inputBackend->initialize();
    }
}

}