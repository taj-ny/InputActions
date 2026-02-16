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
#include "parsers/containers.h"
#include "parsers/core.h"
#include "parsers/utils.h"
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

bool ConfigLoader::load(const ConfigLoadSettings &settings)
{
    try {
        qCDebug(INPUTACTIONS, "Reloading config");
        const auto rawConfig = settings.config.value_or(g_configProvider->currentConfig());

        g_configIssueManager = std::make_shared<ConfigIssueManager>(rawConfig);
        auto config = createConfig(rawConfig);
        activateConfig(std::move(config), true);
    } catch (const ConfigException &e) {
        g_configIssueManager->addIssue(e);
    }

    const auto issues = g_configIssueManager->issues();
    const auto error = std::ranges::find_if(issues, [](const auto *issue) {
        return issue->severity() == ConfigIssueSeverity::Error;
    });

    if (error != issues.end()) {
        if (g_globalConfig->sendNotificationOnError() && !settings.manual) {
            g_notificationManager->sendNotification("Failed to load configuration",
                                                    (*error)->toString(false) + " Run 'inputactions config issues' for more information.");
        }

        return false;
    }

    return true;
}

Config ConfigLoader::createConfig(const QString &raw)
{
    const auto root = Node::create(raw);
    if (!root->isMap()) {
        throw InvalidNodeTypeConfigException(root.get(), NodeType::Map);
    }

    Config config;
    loadMember(config.autoReload, root->at("autoreload"));
    loadMember(config.allowExternalVariableAccess, root->at("external_variable_access"));
    if (const auto *notificationsNode = root->mapAt("notifications")) {
        loadMember(config.sendNotificationOnError, notificationsNode->at("config_error"));
    }
    loadMember(config.libevdevEnabled, root->at("__libevdev_enabled"));
    loadMember(config.deviceRules, root.get());
    loadMember(config.emergencyCombination, root->at("emergency_combination"));

    loadMember(config.keyboardTriggerHandler, root->mapAt("keyboard"));
    loadMember(config.mouseTriggerHandler, root->mapAt("mouse"));
    loadMember(config.pointerTriggerHandler, root->mapAt("pointer"));

    if (const auto *touchpadNode = root->mapAt("touchpad")) {
        config.touchpadTriggerHandlerFactory = [touchpadNode = touchpadNode->shared_from_this()](auto *device) {
            return parseTouchpadTriggerHandler(touchpadNode.get(), device);
        };
        config.touchpadTriggerHandlerFactory(nullptr); // Make sure it doesn't throw
    }
    if (const auto *touchscreenNode = root->mapAt("touchscreen")) {
        config.touchscreenTriggerHandlerFactory = [touchscreenNode = touchscreenNode->shared_from_this()](auto *device) {
            return parseTouchscreenTriggerHandler(touchscreenNode.get(), device);
        };
        config.touchscreenTriggerHandlerFactory(nullptr);
    }

    root->at("anchors"); // Allow users to define anchors somewhere without unused property issues
    root->addUnusedMapPropertyIssues();
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