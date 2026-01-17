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

    int32_t line() const { return m_line; }
    int32_t column() const { return m_column; }
    const char *what() const noexcept override { return m_what.c_str(); }

private:
    int32_t m_line;
    int32_t m_column;
    std::string m_what;
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

class ConfigIssueManager
{
public:
    ConfigIssueManager(QString config = "");

    void addIssue(const Node *node, ConfigIssueSeverity severity, const QString &message);
    void addIssue(int32_t line, int32_t column, ConfigIssueSeverity severity, const QString &message);

    const std::vector<ConfigIssue> &issues() const { return m_issues; }

    QString issuesToString() const;

private:
    std::vector<ConfigIssue> m_issues;
    QString m_config;
};

inline std::shared_ptr<ConfigIssueManager> g_configIssueManager;

}