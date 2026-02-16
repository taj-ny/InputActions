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
#include "ConfigIssue.h"
#include <QStringList>

namespace InputActions
{

ConfigIssueManager::ConfigIssueManager(QString config)
    : m_config(std::move(config))
{
}

std::vector<const ConfigIssue *> ConfigIssueManager::issues() const
{
    std::vector<const ConfigIssue *> result;
    for (const auto &issue : m_issues) {
        result.emplace_back(issue.get());
    }
    return result;
}

QString ConfigIssueManager::issuesToString() const
{
    QString result;
    const auto configLines = m_config.split("\n");
    const auto maxLineNumberLength = QString::number(configLines.size() + 1).size();

    bool hasError{};
    for (const auto &issue : m_issues) {
        if (issue->severity() == ConfigIssueSeverity::Error) {
            hasError = true;
        } else if (dynamic_cast<UnusedPropertyConfigIssue *>(issue.get()) && hasError) {
            // An exception will result in most unused property issues generated after it being false-positives
            continue;
        }

        result += issue->toString() + "\n";

        // Returns line number in the following format: "[number][padding] |"
        const auto lineNumber = [&maxLineNumberLength](int32_t line) {
            line++;
            const auto padding = QString(" ").repeated(maxLineNumberLength - QString::number(line).size());
            return QString("%1%2 | ").arg(QString::number(line), padding);
        };

        if (issue->position().isValid()) {
            const auto lineIndex = issue->position().line();
            const auto column = issue->position().column();
            const auto &line = configLines[lineIndex];
            const auto surroundingLines = 3;

            // Lines before offending line
            for (auto i = std::max(lineIndex - surroundingLines, 0); i < lineIndex; i++) {
                result += QString("%1%2\n").arg(lineNumber(i), configLines[i]);
            }

            // Offending line
            const auto color = issue->colorAnsiSequence();
            result += QString("%1%2\n").arg(lineNumber(lineIndex), AnsiEscapeCode::Color::Bold + color + line + AnsiEscapeCode::Color::Reset);

            // Highlight offending line
            result += QString("%1 | %2").arg(QString(" ").repeated(maxLineNumberLength), AnsiEscapeCode::Color::Bold + color);

            auto highlight = QString("~").repeated(line.size());

            // Remove highlight for leading and trailing whitespace
            for (auto i = 0; i < line.size(); i++) {
                const auto &c = line[i];
                if (c == ' ' || c == '\t') {
                    highlight[i] = ' ';
                    continue;
                }
                break;
            }
            for (auto i = line.size() - 1; i >= 0; i--) {
                const auto &c = line[i];
                if (c == ' ' || c == '\t') {
                    highlight[i] = ' ';
                    continue;
                }
                break;
            }

            if (column < line.size()) {
                highlight[column] = '^';
            } else if (!column) {
                highlight += "^";
            }

            result += highlight + AnsiEscapeCode::Color::Reset + "\n";

            // Lines after offending line
            for (auto i = lineIndex + 1; i < std::min(configLines.size(), static_cast<qsizetype>(lineIndex + surroundingLines + 1)); i++) {
                result += QString("%1%2\n").arg(lineNumber(i), configLines[i]);
            }
        }
        result += "\n";
    }

    if (hasError) {
        result += "At least one error was found, which may have suppressed other issues. Run the command again after fixing it to ensure other problems are "
                  "not missed.";
    } else {
        result.chop(2);
    }
    return result;
}

}