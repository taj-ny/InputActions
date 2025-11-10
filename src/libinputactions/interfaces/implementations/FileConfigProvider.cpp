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

#include "FileConfigProvider.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <fcntl.h>
#include <libinputactions/globals.h>
#include <sys/inotify.h>

namespace InputActions
{

static const QDir INPUTACTIONS_DIR = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/inputactions";

FileConfigProvider::FileConfigProvider()
    : m_path(ensureConfigPath())
{
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

    m_inotifyNotifier = std::make_unique<QSocketNotifier>(m_inotifyFd, QSocketNotifier::Read);
    connect(m_inotifyNotifier.get(), &QSocketNotifier::activated, this, &FileConfigProvider::onReadyRead);

    initWatchers();
    tryReadConfig();
}

FileConfigProvider::~FileConfigProvider()
{
    if (m_inotifyFd != -1) {
        close(m_inotifyFd);
    }
}

const QString &FileConfigProvider::currentPath() const
{
    return m_path;
}

void FileConfigProvider::initWatchers()
{
    m_inotifyWds.push_back(inotify_add_watch(m_inotifyFd, QFileInfo(m_path).dir().path().toStdString().c_str(), IN_CREATE | IN_MODIFY)); // watch dir
    m_inotifyWds.push_back(inotify_add_watch(m_inotifyFd, m_path.toStdString().c_str(), IN_MODIFY | IN_DONT_FOLLOW)); // watch file
    if (!QFile(m_path).symLinkTarget().isEmpty()) {
        m_inotifyWds.push_back(inotify_add_watch(m_inotifyFd, m_path.toStdString().c_str(), IN_MODIFY)); // watch file that link points to
    }
}

void FileConfigProvider::onReadyRead()
{
    std::array<int8_t, 16 * (sizeof(inotify_event) + NAME_MAX + 1)> buffer{};
    auto first = true;
    while (read(m_inotifyFd, &buffer, sizeof(buffer)) > 0) {
        if (!first) {
            continue;
        }
        first = false;

        for (const auto &wd : m_inotifyWds) {
            inotify_rm_watch(m_inotifyFd, wd);
        }
        initWatchers();
        tryReadConfig();
    }
}

void FileConfigProvider::tryReadConfig()
{
    QFile configFile(m_path);
    if (!configFile.open(QIODevice::ReadOnly)) {
        return;
    }

    const auto config = QTextStream(&configFile).readAll();
    if (config != currentConfig()) {
        setConfig(config);
    }
}

QString FileConfigProvider::ensureConfigPath()
{
    if (!INPUTACTIONS_DIR.exists()) {
        INPUTACTIONS_DIR.mkpath(".");
    }

    // pair<path, create>
    std::vector<std::pair<QString, bool>> candidates;
#ifdef DEBUG
    candidates.emplace_back(INPUTACTIONS_DIR.path() + "/config-debug.yaml", false);
#endif
    candidates.emplace_back(INPUTACTIONS_ETC_CONFIG_PATH, false);
    candidates.emplace_back(INPUTACTIONS_DIR.path() + "/config.yaml", true);

    QString finalPath;
    for (const auto &candidate : candidates) {
        const auto &path = candidate.first;
        const auto &create = candidate.second;

        if (QFile::exists(path)) {
            finalPath = path;
            break;
        } else if (create) {
            QFile(path).open(QIODevice::WriteOnly);
            finalPath = path;
            break;
        }
    }
    return finalPath;
}

}