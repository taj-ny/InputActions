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

#include <QObject>
#include <QTimer>

namespace libinputactions
{

class InputBackend;

class Config : public QObject
{
    Q_OBJECT

public:
    Config(InputBackend *backend);
    ~Config() override;

    /**
     * @return std::nullopt if loaded successfully, otherwise the error message.
     */
    std::optional<QString> load(const bool &firstLoad = false);

private:
    void initWatchers();
    void readEvents();

    QString m_path;
    int m_inotifyFd;
    std::vector<int> m_inotifyWds;
    QTimer m_readEventsTimer;

    bool m_autoReload = true;

    InputBackend *m_backend;
};

}