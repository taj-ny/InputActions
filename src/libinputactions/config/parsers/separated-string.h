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

namespace InputActions
{

template<typename T>
inline std::pair<T, T> parseSeparatedString2(const Node *node, char separator)
{
    const auto values = parseSeparatedString2<std::shared_ptr<Node>>(node, separator);
    return std::make_pair(values.first->as<T>(), values.second->as<T>());
}

template<>
inline std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> parseSeparatedString2(const Node *node, char separator)
{
    const auto raw = node->as<QString>();
    const auto split = raw.split(separator);
    if (split.size() != 2) {
        throw InvalidValueConfigException(node, QString("Expected exactly two values separated by '%1'.").arg(separator));
    }

    const auto &first = split[0];
    if (first.isEmpty()) {
        throw InvalidValueConfigException(node, QString("First element of separated string '%1' is empty.").arg(raw));
    }
    if (first.trimmed() != first) {
        throw InvalidValueConfigException(node, QString("First element '%1' of separated string '%2' contains leading or trailing spaces.").arg(first, raw));
    }

    const auto &second = split[1];
    if (second.isEmpty()) {
        throw InvalidValueConfigException(node, QString("Second element of separated string '%2' is empty.").arg(raw));
    }
    if (second.trimmed() != second) {
        throw InvalidValueConfigException(node, QString("Second element '%1' of separated string '%2' contains leading or trailing spaces.").arg(second, raw));
    }

    return std::make_pair(node->substringNodeQuoted(first), node->substringNodeQuoted(second));
}

}