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

#pragma once

#include <QSocketNotifier>
#include <QTimer>
#include <libinputactions/interfaces/ConfigProvider.h>

namespace InputActions
{

static const QString INPUTACTIONS_ETC_CONFIG_PATH = "/etc/inputactions/config.yaml";

class FileConfigProvider : public ConfigProvider
{
    Q_OBJECT

public:
    FileConfigProvider();
    ~FileConfigProvider() override;

    const QString &currentPath() const;

private slots:
    void onReadyRead();
    void onRetryTimerTimeout();

private:
    void initWatchers();
    void tryReadConfig(bool retryIfEmpty = false);

    /**
     * @return Path to the configuration file.
     */
    static QString ensureConfigPath();

    QString m_path;
    QString m_lastContent;
    int m_inotifyFd;
    std::vector<int> m_inotifyWds;
    std::unique_ptr<QSocketNotifier> m_inotifyNotifier;
    QTimer m_retryTimer;
};

}