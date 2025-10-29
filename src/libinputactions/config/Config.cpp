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
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <fcntl.h>
#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/interfaces/NotificationManager.h>
#include <sys/inotify.h>

namespace libinputactions
{

static const QDir INPUTACTIONS_DIR = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/inputactions";

/**
 * Used to detect and prevent infinite compositor crash loops when loading the configuration.
 */
const static QString CRASH_PREVENTION_FILE = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/inputactions_init";

Config::Config()
{
    if (!INPUTACTIONS_DIR.exists()) {
        INPUTACTIONS_DIR.mkpath(".");
    }

    // pair<path, create>
    std::vector<std::pair<QString, bool>> candidates;
#ifdef DEBUG
    candidates.emplace_back(INPUTACTIONS_DIR.path() + "/config-debug.yaml", false);
#endif
    candidates.emplace_back(INPUTACTIONS_DIR.path() + "/config.yaml", true);

    for (const auto &candidate : candidates) {
        const auto &path = candidate.first;
        const auto &create = candidate.second;

        if (QFile::exists(path)) {
            m_path = path;
            break;
        } else if (create) {
            QFile(path).open(QIODevice::WriteOnly);
            m_path = path;
            break;
        }
    }

    m_inotifyFd = inotify_init();
    if (m_inotifyFd == -1) {
        qWarning(INPUTACTIONS, "Failed to initialize config watcher");
        return;
    }
    fcntl(m_inotifyFd, F_SETFD, FD_CLOEXEC);
    if (fcntl(m_inotifyFd, F_SETFL, fcntl(m_inotifyFd, F_GETFL, 0) | O_NONBLOCK) < 0) {
        qWarning(INPUTACTIONS, "Failed to initialize config watcher (fcntl failed)");
        return;
    }
    initWatchers();

    connect(&m_readEventsTimer, &QTimer::timeout, this, &Config::readEvents);
    m_readEventsTimer.setInterval(500);
    m_readEventsTimer.start();
}

Config::~Config()
{
    if (m_inotifyFd != -1) {
        close(m_inotifyFd);
    }
}

std::optional<QString> Config::load(bool firstLoad)
{
    std::optional<QString> error;
    auto sendNotificationOnError = true;

    if (!QFile::exists(CRASH_PREVENTION_FILE) || !firstLoad) {
        QFile(CRASH_PREVENTION_FILE).open(QIODevice::WriteOnly);
        try {
            QFile configFile(m_path);
            if (!configFile.open(QIODevice::ReadOnly)) {
                throw std::runtime_error("Failed to open the configuration file");
            }
            const auto contents = QTextStream(&configFile).readAll();
            if (contents == m_lastContents) {
                // No change, don't spam the user
                sendNotificationOnError = false;
            }
            m_lastContents = contents;
            qCDebug(INPUTACTIONS, "Reloading config");

            const auto config = YAML::Load(contents.toStdString());
            m_autoReload = config["autoreload"].as<bool>(true);

            m_sendNotificationOnError = true;
            if (const auto &notificationsNode = config["notifications"]) {
                if (const auto &configErrorNode = notificationsNode["config_error"]) {
                    m_sendNotificationOnError = configErrorNode.as<bool>();
                }
            }

            auto keyboardTriggerHandler = config["keyboard"].as<std::unique_ptr<KeyboardTriggerHandler>>(nullptr);
            auto mouseTriggerHandler = config["mouse"].as<std::unique_ptr<MouseTriggerHandler>>(nullptr);
            auto pointerTriggerHandler = config["pointer"].as<std::unique_ptr<PointerTriggerHandler>>(nullptr);
            std::function<std::unique_ptr<TouchpadTriggerHandler>(InputDevice * device)> touchpadTriggerHandlerFactory;
            if (const auto &touchpadNode = config["touchpad"]) {
                touchpadTriggerHandlerFactory = [touchpadNode](auto *device) {
                    return YAML::asTouchpadTriggerHandler(touchpadNode, device);
                };
                touchpadTriggerHandlerFactory(nullptr); // Make sure it doesn't throw
            }

            auto deviceRules = config.as<std::vector<InputDeviceRule>>();

            if (auto *libevdev = dynamic_cast<LibevdevComplementaryInputBackend *>(g_inputBackend.get())) {
                if (const auto &pollingIntervalNode = config["__libevdev_polling_interval"]) {
                    libevdev->setPollingInterval(pollingIntervalNode.as<uint32_t>());
                }
                if (const auto &enabledNode = config["__libevdev_enabled"]) {
                    libevdev->setEnabled(enabledNode.as<bool>());
                }
            }

            g_inputBackend->reset();
            g_inputBackend->m_keyboardTriggerHandler = std::move(keyboardTriggerHandler);
            g_inputBackend->m_mouseTriggerHandler = std::move(mouseTriggerHandler);
            g_inputBackend->m_pointerTriggerHandler = std::move(pointerTriggerHandler);
            g_inputBackend->m_touchpadTriggerHandlerFactory = std::move(touchpadTriggerHandlerFactory);
            g_inputBackend->m_deviceRules = std::move(deviceRules);
            g_inputBackend->initialize();
        } catch (const std::exception &e) {
            error = QString("Failed to load configuration: %1").arg(QString::fromStdString(e.what()));
        }
    } else {
        error = "Configuration was not loaded automatically due to a crash.";
    }

    if (error) {
        qCCritical(INPUTACTIONS).noquote() << error.value();
        if (sendNotificationOnError && m_sendNotificationOnError) {
            g_notificationManager->sendNotification("Failed to load configuration", error.value());
        }
    }
    QFile::remove(CRASH_PREVENTION_FILE);
    return error;
}

void Config::initWatchers()
{
    m_inotifyWds.push_back(inotify_add_watch(m_inotifyFd, QFileInfo(m_path).dir().path().toStdString().c_str(), IN_CREATE | IN_MODIFY)); // watch dir
    m_inotifyWds.push_back(inotify_add_watch(m_inotifyFd, m_path.toStdString().c_str(), IN_MODIFY | IN_DONT_FOLLOW)); // watch file
    if (!QFile(m_path).symLinkTarget().isEmpty()) {
        m_inotifyWds.push_back(inotify_add_watch(m_inotifyFd, m_path.toStdString().c_str(), IN_MODIFY)); // watch file that link points to
    }
}

void Config::readEvents()
{
    auto first = true;
    char buffer[512];
    while (read(m_inotifyFd, &buffer, sizeof(buffer)) > 0) {
        if (!first) {
            continue;
        }
        first = false;

        for (const auto &wd : m_inotifyWds) {
            inotify_rm_watch(m_inotifyFd, wd);
        }
        initWatchers();

        if (m_autoReload) {
            load();
        }
    }
}

}
