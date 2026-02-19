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

#pragma once

#include <QString>
#include <memory>
#include <optional>

namespace InputActions
{

struct Config;

struct ConfigLoadSettings
{
    /**
     * If not set, the config returned by ConfigProvider will be used.
     */
    std::optional<QString> config;
    /**
     * Whether the reload was manually initiated using the control tool.
     */
    bool manual{};
};

class ConfigLoader
{
public:
    /**
     * @return Whether the operation was successful. Errors may be obtained from ConfigIssueManager.
     */
    bool load(const ConfigLoadSettings &settings = {});

    /**
     * Loads an empty config with default values without initializing any components.
     */
    void loadEmpty();

private:
    Config createConfig(const QString &raw);
    void activateConfig(Config config, bool initialize);
};

inline std::shared_ptr<ConfigLoader> g_configLoader;

}