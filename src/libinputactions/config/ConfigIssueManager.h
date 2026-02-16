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

#include "ConfigIssue.h"
#include <QString>
#include <memory>

namespace InputActions
{

class Node;

class ConfigIssueManager
{
public:
    ConfigIssueManager(QString config = "");

    void addIssue(const ConfigIssue &issue)
    {
        if (std::ranges::any_of(m_issues, [&issue](const auto &x) {
                return *x == issue;
            })) {
            return;
        }

        const auto pos = std::ranges::lower_bound(m_issues, &issue, [](const auto &a, const auto &b) {
            // severity desc, line asc, column asc
            if (a->severity() != b->severity()) {
                return a->severity() > b->severity();
            }
            if (a->position().line() != b->position().line()) {
                return a->position().line() < b->position().line();
            }
            return a->position().column() < b->position().column();
        });

        m_issues.insert(pos, issue.copy());
    }

    template<typename T>
        requires std::is_base_of_v<ConfigIssue, T>
    const T *findIssueByType() const
    {
        for (const auto &issue : m_issues) {
            if (const auto *ptr = dynamic_cast<const T *>(issue.get())) {
                return ptr;
            }
        }
        return {};
    }

    std::vector<const ConfigIssue *> issues() const;

    QString issuesToString() const;

private:
    std::vector<std::unique_ptr<ConfigIssue>> m_issues;
    QString m_config;
};

inline std::shared_ptr<ConfigIssueManager> g_configIssueManager;

}