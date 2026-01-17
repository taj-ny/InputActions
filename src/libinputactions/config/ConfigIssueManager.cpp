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

#include "ConfigIssueManager.h"
#include "../../common/ansi-escape-codes.h"
#include "Node.h"
#include <QStringList>

namespace InputActions
{

ConfigIssueManager::ConfigIssueManager(QString config)
    : m_config(std::move(config))
{
}

void ConfigIssueManager::addIssue(const Node *node, ConfigIssueSeverity severity, const QString &message)
{
    addIssue(node->line(), node->column(), severity, message);
}

void ConfigIssueManager::addIssue(int32_t line, int32_t column, ConfigIssueSeverity severity, const QString &message)
{
    ConfigIssue issue(line, column, severity, message);
    if (std::ranges::contains(m_issues, issue)) {
        return;
    }

    const auto pos = std::ranges::lower_bound(m_issues, issue, [](const auto &a, const auto &b) {
        // severity desc, line asc, column asc
        if (a.severity() != b.severity()) {
            return a.severity() > b.severity();
        }
        if (a.line() != b.line()) {
            return a.line() < b.line();
        }
        return a.column() < b.column();
    });
    m_issues.emplace(pos, line, column, severity, message);
}

QString ConfigIssueManager::issuesToString() const
{
    QString result;
    const auto configLines = m_config.split("\n");
    const auto maxLineNumberLength = QString::number(m_config.size() + 1).size();

    bool hasError{};
    for (const auto &issue : m_issues) {
        if (issue.severity() == ConfigIssueSeverity::Error) {
            hasError = true;
        } else if (issue.severity() == ConfigIssueSeverity::UnusedProperty && hasError) {
            // An error (exception) will result in most unused property issues generated after it being false-positives
            continue;
        }

        const auto hasPosition = issue.line() != -1;
        QString positionString;
        if (hasPosition) {
            positionString = QString("%1:%2: ").arg(QString::number(issue.line() + 1), QString::number(issue.column() + 1));
        }

        QString severityString;
        QString color;
        switch (issue.severity()) {
            case ConfigIssueSeverity::Deprecation:
                severityString = "deprecation";
                color = AnsiEscapeCode::Color::Blue;
                break;
            case ConfigIssueSeverity::UnusedProperty:
                severityString = "unused_property";
                color = AnsiEscapeCode::Color::Blue;
                break;
            case ConfigIssueSeverity::Warning:
                severityString = "warning";
                color = AnsiEscapeCode::Color::Yellow;
                break;
            case ConfigIssueSeverity::Error:
                severityString = "error";
                color = AnsiEscapeCode::Color::Red;
                break;
        }

        // Message
        result += QString("%1%2: %3\n")
                      .arg(positionString, AnsiEscapeCode::Color::Bold + color + severityString + AnsiEscapeCode::Color::Reset, issue.message());

        // Returns line number in the following format: "[number][padding] | "
        const auto lineNumber = [&maxLineNumberLength](int32_t line) {
            line++;
            const auto padding = QString(" ").repeated(maxLineNumberLength - QString::number(line).size());
            return QString("%1%2 | ").arg(QString::number(line), padding);
        };

        if (hasPosition) {
            // Lines before offending line
            for (auto line = std::max(issue.line() - 5, 0); line < issue.line(); line++) {
                result += QString("%1%2\n").arg(lineNumber(line), configLines[line]);
            }

            // Offending line
            result += QString("%1%2\n").arg(lineNumber(issue.line()),
                                            AnsiEscapeCode::Color::Bold + color + configLines[issue.line()] + AnsiEscapeCode::Color::Reset);

            // Highlight offending line
            result += QString("%1 | %2").arg(QString(" ").repeated(maxLineNumberLength), AnsiEscapeCode::Color::Bold + color);
            auto skipWhitespace = true;
            for (qsizetype i = 0; i < configLines[issue.line()].size(); i++) {
                const auto c = configLines[issue.line()][i];
                const auto isWhitespace = c == ' ' || c == '\t';

                if (i == issue.column()) {
                    result += "^";
                    continue;
                }

                if (skipWhitespace) {
                    if (isWhitespace) {
                        result += " ";
                        continue;
                    }
                    skipWhitespace = false;
                }

                result += "~";
            }
            result += AnsiEscapeCode::Color::Reset + "\n";

            // Lines after offending line
            for (auto line = issue.line() + 1; line < std::min(configLines.size(), static_cast<qsizetype>(issue.line() + 6)); line++) {
                result += QString("%1%2\n").arg(lineNumber(line), configLines[line]);
            }
        }
        result += "\n";
    }

    if (hasError) {
        result += "At least one error was found, which may have suppressed other issues. Run the command again after fixing it to ensure other problems are "
                  "not missed.";
    }
    return result;
}

ConfigIssue::ConfigIssue(int32_t line, int32_t column, ConfigIssueSeverity severity, QString message)
    : m_line(line)
    , m_column(column)
    , m_severity(severity)
    , m_message(std::move(message))
{
}

ConfigParserException::ConfigParserException(const Node *node, const QString &message)
    : m_line(node->line())
    , m_column(node->column())
    , m_what(message.toStdString())
{
    node->disableMapAccessCheck();
}

}