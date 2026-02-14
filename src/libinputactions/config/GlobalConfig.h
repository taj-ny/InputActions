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

namespace InputActions
{

class GlobalConfig
{
public:
    /**
     * Allow external programs to read InputActions variables.
     */
    bool allowExternalVariableAccess() const { return m_allowExternalVariableAccess; }
    void setAllowExternalVariableAccess(bool value) { m_allowExternalVariableAccess = value; }

    bool autoReload() const { return m_autoReload; }
    void setAutoReload(bool value) { m_autoReload = value; }

    bool sendNotificationOnError() const { return m_sendNotificationOnError; }
    void setSendNotificationOnError(bool value) { m_sendNotificationOnError = value; }

private:
    // Default values defined in Config
    bool m_allowExternalVariableAccess{};
    bool m_autoReload{};
    bool m_sendNotificationOnError{};
};

inline std::shared_ptr<GlobalConfig> g_globalConfig;

}