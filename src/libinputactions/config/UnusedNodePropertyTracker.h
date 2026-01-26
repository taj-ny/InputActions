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
#include <yaml-cpp/yaml.h>

namespace InputActions
{

class Node;

class UnusedNodePropertyTracker
{
public:
    UnusedNodePropertyTracker() = default;

    bool enabled() const { return m_enabled; }
    void setEnabled(bool value) { m_enabled = value; }

    void registerPropertyAccess(QString property);
    void check(const Node *node);

private:
    std::set<QString> m_accessedProperties;
    bool m_enabled = true;
};

}