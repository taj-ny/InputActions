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
#include "TextPosition.h"
#include "parsers/NodeParser.h"
#include <QString>
#include <memory>
#include <yaml-cpp/yaml.h>

namespace InputActions
{

enum class NodeType
{
    Null,
    Map,
    Scalar,
    Sequence
};

/**
 * Wrapper for yaml-cpp's Node. Each node has exactly one wrapper.
 *
 * Call shared_from_this to get a shared pointer to this node.
 */
class Node : public std::enable_shared_from_this<Node>
{
    struct Private
    {
        explicit Private() = default;
    };

public:
    Node(Private, NodeType type);
    Node(Private, YAML::Node node);
    ~Node();

    /**
     * Constructs an empty node of the specified type.
     */
    static std::shared_ptr<Node> create(NodeType type);
    /**
     * @param node Must not have an existing wrapper.
     */
    static std::shared_ptr<Node> create(YAML::Node node);
    /**
     * @throws YamlCppConfigException
     */
    static std::shared_ptr<Node> create(const QString &s);

    const TextPosition &position() const { return m_position; }
    void setPosition(TextPosition value) { m_position = value; }

    NodeType type() const { return m_type; }
    bool isNull() const;
    bool isMap() const;
    bool isScalar() const;
    bool isSequence() const;

    /**
     * Whether this node is a substring of a scalar node.
     */
    bool isSubstring() const;
    /**
     * @return Empty string if the node is not a substring.
     */
    QString substring() const;

    const QString &tag() const { return m_tag; }

    const YAML::Node &raw() const { return m_node; }

    /**
     * Parses the node's value as the specified type.
     * @throws ConfigException
     */
    template<typename T>
    T as(bool allowImplicitConversionToSequence = false) const
    {
        if (isNull()) {
            throw InvalidValueConfigException(this, "Value is null.");
        }

        T result;
        if (allowImplicitConversionToSequence) {
            m_allowImplicitConversionToSequence = true;
        }
        NodeParser<T>::parse(this, result);
        m_allowImplicitConversionToSequence = false;
        return result;
    }

    /**
     * Converts a substring of this node to a node. If the result is always going to be a string, use substringNodeQuoted.
     * @return May be a non-string.
     */
    std::shared_ptr<Node> substringNode(const QString &substring) const;
    /**
     * Converts a substring of this node to a string node.
     * @return Always a string node.
     */
    std::shared_ptr<Node> substringNodeQuoted(const QString &substring) const;

    /**
     * @return The node at this map node's specified key or nullptr if no node exists at the specified key.
     * @throws MissingRequiredPropertyConfigException No node exists at the specified key and required is true.
     */
    const Node *at(const char *key, bool required = false) const;
    /**
     * @return The map node at this map node's specified key or nullptr if no node exists at the specified key.
     * @throws InvalidNodeTypeConfigException The node at the specified key is not a map.
     * @throws MissingRequiredPropertyConfigException No node exists at the specified key and required is true.
     */
    const Node *mapAt(const char *key, bool required = false) const;

    /**
     * @return Items of this sequence node.
     * @throws InvalidNodeTypeConfigException The node is not a sequence.
     */
    std::vector<const Node *> sequenceItems() const;

    /**
     * @return String keys and node values of this map node. Returned nodes are not marked as used.
     * @throws InvalidNodeTypeConfigException The node is not a map.
     */
    std::map<QString, const Node *> mapItems() const;
    /**
     * @return Node keys and node values of this map node. Returned nodes are not marked as used.
     * @throws InvalidNodeTypeConfigException The node is not a map.
     */
    std::map<const Node *, const Node *> mapItemsRawKeys() const;

    /**
     * Adds an item to this map node.
     */
    void addMapItem(std::shared_ptr<const Node> key, std::shared_ptr<const Node> value);
    /**
     * Recursively goes through all nodes and adds UnusedPropertyConfigIssues for unused map values.
     * This method should only be called on the root node.
     */
    void addUnusedMapPropertyIssues() const;
    /**
     * Marks this node of a map as used.
     */
    void markUsed() const { m_used = true; }

private:
    YAML::Node m_node;
    NodeType m_type;
    QString m_tag;
    TextPosition m_position;
    std::optional<QString> m_substringValue;

    mutable bool m_allowImplicitConversionToSequence = false;
    mutable bool m_used{};

    std::map<std::shared_ptr<const Node>, std::shared_ptr<const Node>> m_mapItems;
    std::vector<std::shared_ptr<Node>> m_sequenceItems;
};

}