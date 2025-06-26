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

#include <libinputactions/input/backends/LibevdevComplementaryInputBackend.h>
#include <libinputactions/yaml_convert.h>

#include <QDir>
#include <QFile>
#include <QStandardPaths>

#include <fcntl.h>
#include <sys/inotify.h>

namespace libinputactions
{

static const QDir INPUTACTIONS_DIR = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/inputactions";
static const QString CONFIG_PATH = INPUTACTIONS_DIR.path() + "/config.yaml";
static const QString LEGACY_CONFIG_PATH = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kwingestures.yml";

/**
 * Used to detect and prevent infinite compositor crash loops when loading the configuration.
 */
const static QString CRASH_PREVENTION_FILE = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/inputactions_init";

Config::Config(InputBackend *backend)
    : m_backend(backend)
{
    if (!INPUTACTIONS_DIR.exists()) {
        INPUTACTIONS_DIR.mkpath(".");
    }
    if (QFile::exists(LEGACY_CONFIG_PATH) && !QFile::exists(CONFIG_PATH)) {
        QFile::copy(LEGACY_CONFIG_PATH, CONFIG_PATH);
    }

    // pair<path, create>
    std::vector<std::pair<QString, bool>> candidates;
#ifdef DEBUG
    candidates.emplace_back(INPUTACTIONS_DIR.path() + "/config-debug.yaml", false);
#endif
    candidates.emplace_back(CONFIG_PATH, true);

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

std::optional<QString> Config::load(const bool &firstLoad)
{
    qCDebug(INPUTACTIONS, "Reloading config");
    std::optional<QString> error;
    if (!QFile::exists(CRASH_PREVENTION_FILE) || !firstLoad) {
        QFile(CRASH_PREVENTION_FILE).open(QIODevice::WriteOnly);
        try {
            const auto config = YAML::LoadFile(m_path.toStdString());
            m_autoReload = config["autoreload"].as<bool>(true);

            m_backend->reset();
            for (auto &eventHandler : config.as<std::vector<std::unique_ptr<InputEventHandler>>>()) {
                m_backend->addEventHandler(std::move(eventHandler));
            }
            if (const auto &touchpadNode = config["touchpad"]) {
                if (const auto &devicesNode = touchpadNode["devices"]) {
                    for (auto it = devicesNode.begin(); it != devicesNode.end(); it++) {
                        m_backend->addCustomDeviceProperties(it->first.as<QString>(), it->second.as<InputDeviceProperties>());
                    }
                }
            }

            if (auto *libevdev = dynamic_cast<LibevdevComplementaryInputBackend *>(m_backend)) {
                if (const auto &pollingIntervalNode = config["__libevdev_polling_interval"]) {
                    libevdev->setPollingInterval(pollingIntervalNode.as<uint32_t>());
                }
                if (const auto &enabledNode = config["__libevdev_enabled"]) {
                    libevdev->setEnabled(enabledNode.as<bool>());
                }
            }

            m_backend->initialize();
        } catch (const YAML::Exception &e) {
            const auto message = QString("Failed to load configuration: %1 (line %2, column %3)")
                .arg(QString::fromStdString(e.msg), QString::number(e.mark.line), QString::number(e.mark.column));
            qCritical(INPUTACTIONS) << message;
            error = message;
        }
    } else {
        qCWarning(INPUTACTIONS) << "Configuration was not loaded automatically due to a crash.";
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