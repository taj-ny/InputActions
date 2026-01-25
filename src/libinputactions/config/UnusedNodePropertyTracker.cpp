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

#include "UnusedNodePropertyTracker.h"
#include "Node.h"
#include <libinputactions/config/ConfigIssueManager.h>
#include <yaml-cpp/yaml.h>

namespace InputActions
{

void UnusedNodePropertyTracker::check(const Node *node)
{
    const auto &raw = node->raw();
    for (auto it = raw.begin(); it != raw.end(); ++it) {
        if (const auto key = QString::fromStdString(it->first.as<std::string>("")); !key.isEmpty()) {
            if (!m_accessedProperties.contains(key)) {
                YAML::Mark mark = it->first.Mark();
                g_configIssueManager->addIssue(UnusedPropertyConfigIssue({mark.line, mark.column}, key));
            }
        }
    }
}

void UnusedNodePropertyTracker::registerPropertyAccess(QString property)
{
    m_accessedProperties.insert(std::move(property));
}

}