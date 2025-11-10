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

#include "Config.h"
#include "yaml.h"
#include <QFile>
#include <QStandardPaths>
#include <libinputactions/globals.h>
#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/interfaces/ConfigProvider.h>
#include <libinputactions/interfaces/InputEmitter.h>
#include <libinputactions/interfaces/NotificationManager.h>

namespace InputActions
{

/**
 * Used to detect and prevent infinite compositor crash loops when loading the configuration.
 */
const static QString CRASH_PREVENTION_FILE = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/inputactions_init";

std::optional<QString> Config::load(const QString &config, bool preventCrashLoops)
{
    std::optional<QString> error;

    if (!QFile::exists(CRASH_PREVENTION_FILE) || !preventCrashLoops) {
        std::ignore = QFile(CRASH_PREVENTION_FILE).open(QIODevice::WriteOnly);
        try {
            qCDebug(INPUTACTIONS, "Reloading config");

            auto newConfig = std::make_shared<Config>();
            const auto root = YAML::Load(config.toStdString());
            newConfig->setAutoReload(root["autoreload"].as<bool>(true));

            if (const auto &notificationsNode = root["notifications"]) {
                if (const auto &configErrorNode = notificationsNode["config_error"]) {
                    newConfig->setSendNotificationOnError(configErrorNode.as<bool>());
                }
            }

            g_config = newConfig;

            auto keyboardTriggerHandler = root["keyboard"].as<std::unique_ptr<KeyboardTriggerHandler>>(nullptr);
            auto mouseTriggerHandler = root["mouse"].as<std::unique_ptr<MouseTriggerHandler>>(nullptr);
            auto pointerTriggerHandler = root["pointer"].as<std::unique_ptr<PointerTriggerHandler>>(nullptr);
            std::function<std::unique_ptr<TouchpadTriggerHandler>(InputDevice * device)> touchpadTriggerHandlerFactory;
            if (const auto &touchpadNode = root["touchpad"]) {
                touchpadTriggerHandlerFactory = [touchpadNode](auto *device) {
                    return YAML::asTouchpadTriggerHandler(touchpadNode, device);
                };
                touchpadTriggerHandlerFactory(nullptr); // Make sure it doesn't throw
            }

            auto deviceRules = root.as<std::vector<InputDeviceRule>>();

            if (auto *libevdev = dynamic_cast<LibevdevComplementaryInputBackend *>(g_inputBackend.get())) {
                if (const auto &pollingIntervalNode = root["__libevdev_polling_interval"]) {
                    libevdev->setPollingInterval(pollingIntervalNode.as<uint32_t>());
                }
                if (const auto &enabledNode = root["__libevdev_enabled"]) {
                    libevdev->setEnabled(enabledNode.as<bool>());
                }
            }

            g_inputBackend->reset();
            g_inputEmitter->reset(); // Okay because required keys are not cleared

            g_inputBackend->m_keyboardTriggerHandler = std::move(keyboardTriggerHandler);
            g_inputBackend->m_mouseTriggerHandler = std::move(mouseTriggerHandler);
            g_inputBackend->m_pointerTriggerHandler = std::move(pointerTriggerHandler);
            g_inputBackend->m_touchpadTriggerHandlerFactory = std::move(touchpadTriggerHandlerFactory);
            g_inputBackend->m_deviceRules = std::move(deviceRules);

            g_inputEmitter->initialize();
            g_inputBackend->initialize();
        } catch (const std::exception &e) {
            error = QString("Failed to load configuration: %1").arg(QString::fromStdString(e.what()));
        }
    } else {
        error = "Configuration was not loaded automatically due to a crash.";
    }

    if (error) {
        qCCritical(INPUTACTIONS).noquote() << error.value();
        if (g_config->sendNotificationOnError()) {
            g_notificationManager->sendNotification("Failed to load configuration", error.value());
        }
    }
    QFile::remove(CRASH_PREVENTION_FILE);
    return error;
}

std::optional<QString> Config::load(bool preventCrashLoops)
{
    return load(g_configProvider->currentConfig(), preventCrashLoops);
}

}