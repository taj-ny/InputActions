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

#include "TextPosition.h"
#include "libinputactions/config/ConfigIssue.h"
#include "parsers/NodeParser.h"
#include <QString>
#include <yaml-cpp/yaml.h>

namespace InputActions
{

class ConfigParser;
class UnusedNodePropertyTracker;

enum class NodeType
{
    Null,
    Map,
    Scalar,
    Sequence
};

/**
 * Minimal wrapper for yaml-cpp's Node.
 */
class Node
{
public:
    Node(YAML::Node node, const Node *substringParent = nullptr);
    ~Node();

    static Node load(const QString &s);

    const TextPosition &position() const { return m_position; }
    void setPosition(TextPosition value) { m_position = value; }

    NodeType type() const;
    bool isMap() const;
    bool isScalar() const;
    bool isSequence() const;

    QString tag() const;

    bool allowImplicitConversionToSequence() const { return m_allowImplicitConversionToSequence; }

    const YAML::Node &raw() const { return m_node; }
    /**
     * Can be manipulated.
     */
    YAML::Node &raw() { return m_node; }

    /**
     * Converts this node into a sequence node containing only this node. If this node is a sequence, the same node is returned with no changes.
     */
    Node asSequence() const;
    /**
     * Clones this node using YAML::Clone.
     */
    Node clone() const;

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

    Node substringNode(const QString &substring) const;

    UnusedNodePropertyTracker *unusedPropertyTracker() const { return m_unusedPropertyTracker.get(); }

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
     * @param disableUnusedPropertyCheck Disable the unused property check for all returned wrappers.
     * @return Children of this sequence node.
     * @throws ConfigParserException The node is not a sequence.
     */
    std::vector<std::shared_ptr<const Node>> sequenceChildren(bool disableUnusedPropertyCheck = false) const;
    /**
     * @param disableUnusedPropertyCheck Disable the unused property check for all returned wrappers.
     * @return Children of this sequence node.
     * @throws ConfigParserException The node is not a sequence.
     */
    std::vector<std::shared_ptr<Node>> sequenceChildren(bool disableUnusedPropertyCheck = false);

    /**
     * @return Keys and values of this map node.
     * @throws ConfigParserException The node is not a map.
     */
    std::map<std::shared_ptr<const Node>, std::shared_ptr<const Node>> mapChildren() const;

private:
    YAML::Node m_node;
    TextPosition m_position;
    bool m_allowImplicitConversionToSequence = false;
    bool m_isSubstringNode = false;

    std::shared_ptr<UnusedNodePropertyTracker> m_unusedPropertyTracker;
};

}