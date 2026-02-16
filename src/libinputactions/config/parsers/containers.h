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

#include "NodeParser.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>
#include <set>
#include <vector>

namespace InputActions
{

class InputActionItem;
class InputDeviceRule;

template<typename T>
struct NodeParser<std::set<T>>
{
    static void parse(const Node *node, std::set<T> &result)
    {
        const auto items = node->sequenceItems();
        for (size_t i = 0; i < items.size(); i++) {
            const auto *item = items[i];

            const auto value = item->as<T>();
            if (result.contains(value)) {
                throw DuplicateSetItemConfigException(item, i);
            }

            result.insert(value);
        }
    }
};

template<typename T>
    requires(!std::is_same_v<T, InputActionItem> && !std::is_same_v<T, InputDeviceRule>)
struct NodeParser<std::vector<T>>
{
    static void parse(const Node *node, std::vector<T> &result)
    {
        for (const auto *item : node->sequenceItems()) {
            result.push_back(item->as<T>());
        }
    }
};

}