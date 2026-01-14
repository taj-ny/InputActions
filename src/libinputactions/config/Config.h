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

class Node;

class ConfigParserException : public std::exception
{
public:
    ConfigParserException(const Node *node, const QString &message);
};

enum class ConfigIssueSeverity
{
    UnusedProperty,
    Deprecation,
    Warning,
    Error,
};

class ConfigIssue
{
public:
    ConfigIssue(int32_t line, int32_t column, ConfigIssueSeverity severity, QString message);

    int32_t line() const { return m_line; }
    int32_t column() const { return m_column; }
    ConfigIssueSeverity severity() const { return m_severity; }
    const QString &message() const { return m_message; }

    bool operator==(const ConfigIssue &) const = default;

private:
    int32_t m_line;
    int32_t m_column;
    ConfigIssueSeverity m_severity;
    QString m_message;
};

class Config
{
public:
    /**
     * Use issuesToString to get detailed information about issues.
     * @return Whether the operation was successful.
     */
    static bool load(const QString &config, bool preventCrashLoops = false);
    /**
     * Use issuesToString to get detailed information about issues.
     * @return Whether the operation was successful.
     */
    static bool load(bool preventCrashLoops = false);

    bool autoReload() const { return m_autoReload; }
    void setAutoReload(bool value) { m_autoReload = value; }

    bool sendNotificationOnError() const { return m_sendNotificationOnError; }
    void setSendNotificationOnError(bool value) { m_sendNotificationOnError = value; }

    void addIssue(const Node *node, ConfigIssueSeverity severity, const QString &message);
    void addIssue(int32_t line, int32_t column, ConfigIssueSeverity severity, const QString &message);
    const std::vector<ConfigIssue> &issues() const { return m_issues; }

    QString issuesToString(QString config = {}) const;

private:
    bool m_autoReload = true;
    bool m_sendNotificationOnError = true;

    std::vector<ConfigIssue> m_issues;
};

inline std::shared_ptr<Config> g_config;

}