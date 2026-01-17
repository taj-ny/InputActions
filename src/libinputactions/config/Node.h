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

#include "parsers/NodeParser.h"
#include <QString>
#include <yaml-cpp/yaml.h>

namespace InputActions
{

class ConfigParser;

/**
 * Minimal read-only wrapper for yaml-cpp's Node that tracks accessed map keys for unused property detection.
 */
class Node
{
public:
    Node(YAML::Node node = {});
    Node(QString text, const Node *parentNode);
    ~Node();

    /**
     * Parses the node's value as the specified type.
     * @throws ConfigParseException
     * @throws YAML::Exception
     */
    template<typename T>
    T as() const
    {
        T result;
        NodeParser<T>::parse(this, result);
        return result;
    }

    /**
     * Permanently disables map key access tracking for this wrapper instance.
     */
    void disableMapAccessCheck() const { m_mapAccessCheckEnabled = false; }
    /**
     * Permanently disables map key access tracking in this wrapper instance for the specified key.
     */
    void disableMapAccessCheck(const char *key) const { m_accessedMapNodes.insert(key); }

    /**
     * @return The node at this map node's specified key or nullptr if no node exists at the specified key.
     * @throws ConfigParserException If required is true and no node exists at the specified key.
     */
    std::shared_ptr<const Node> at(const char *key, bool required = false) const;
    /**
     * @return The map node at this map node's specified key or nullptr if no node exists at the specified key.
     * @throws ConfigParserException The node at the specified key is not a map, or required is true and the node does not exist.
     */
    std::shared_ptr<const Node> mapAt(const char *key, bool required = false) const;

    /**
     * Converts this node into a sequence node containing only this node. If this node is a sequence, the same node is returned with no changes.
     */
    Node asSequence() const;
    /**
     * @return This node cloned with YAML::Clone.
     */
    Node clone() const;

    bool isSequence() const;
    /**
     * @return Children of this sequence node.
     * @throws ConfigParserException The node is not a sequence.
     */
    std::vector<std::shared_ptr<const Node>> sequenceChildren(bool disableMapAccessCheck = false) const;
    /**
     * @return Children of this sequence node.
     * @throws ConfigParserException The node is not a sequence.
     */
    std::vector<std::shared_ptr<Node>> sequenceChildren(bool disableMapAccessCheck = false);

    bool isMap() const;
    /**
     * @return Keys and values of this sequence node.
     * @throws ConfigParserException The node is not a map.
     */
    std::map<std::shared_ptr<const Node>, std::shared_ptr<const Node>> mapChildren() const;

    int32_t line() const { return m_line; }
    int32_t column() const { return m_column; }
    void setPosition(int32_t line, int32_t column)
    {
        m_line = line;
        m_column = column;
    }

    const YAML::Node *raw() const { return &m_node; }
    /**
     * Can be manipulated.
     */
    YAML::Node *raw() { return &m_node; }

    operator bool() const;

private:
    YAML::Node m_node;
    int32_t m_line{};
    int32_t m_column{};

    mutable std::set<QString> m_accessedMapNodes;
    mutable bool m_mapAccessCheckEnabled = true;
};

}