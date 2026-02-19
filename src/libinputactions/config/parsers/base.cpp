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

#include "NodeParser.h"
#include <libinputactions/config/ConfigIssue.h>
#include <libinputactions/config/Node.h>

namespace InputActions
{

#define NODEPARSER_BASE(T, name)                                                            \
    template<>                                                                              \
    void NodeParser<T>::parse(const Node *node, T &result)                                  \
    {                                                                                       \
        if (!node->isScalar()) {                                                            \
            throw InvalidNodeTypeConfigException(node, NodeType::Scalar);                   \
        }                                                                                   \
                                                                                            \
        try {                                                                               \
            result = node->raw().as<T>();                                                   \
        } catch (const YAML::Exception &) {                                                 \
            throw InvalidValueConfigException(node, QString("Value is not %1.").arg(name)); \
        }                                                                                   \
    }

NODEPARSER_BASE(bool, "a boolean");
NODEPARSER_BASE(int8_t, "an 8-bit signed integer");
NODEPARSER_BASE(uint8_t, "an 8-bit unsigned integer");
NODEPARSER_BASE(uint32_t, "a 32-bit unsigned integer");
NODEPARSER_BASE(uint64_t, "a 64-bit unsigned integer");
NODEPARSER_BASE(qreal, "a number");
NODEPARSER_BASE(std::string, "a string");

}