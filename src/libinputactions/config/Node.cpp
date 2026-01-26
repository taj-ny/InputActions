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

#include "Node.h"
#include "ConfigIssue.h"
#include "ConfigIssueManager.h"
#include "config/TextPosition.h"
#include "UnusedNodePropertyTracker.h"
#include <QLoggingCategory>

namespace InputActions
{

Node::Node(YAML::Node node, const Node *substringParent)
    : m_node(node)
    , m_position(node.Mark().line, node.Mark().column)
{
    if (substringParent && substringParent->m_isSubstringNode) {
        m_position = substringParent->position();
    }

    // if (isMap()) {
        m_unusedPropertyTracker = std::make_shared<UnusedNodePropertyTracker>();
    // }
}

Node::~Node()
{
    if (m_unusedPropertyTracker && isMap() && m_unusedPropertyTracker->enabled()) {
        m_unusedPropertyTracker->check(this);
    }
}

Node Node::load(const QString &s)
{
    try {
        return {YAML::Load(s.toStdString())};
    } catch (const YAML::Exception &e) {
        throw YamlCppConfigException({e.mark.line, e.mark.column}, e.what());
    }
}

std::shared_ptr<const Node> Node::at(const char *key, bool required) const
{
    if (m_unusedPropertyTracker) {
        m_unusedPropertyTracker->registerPropertyAccess(key);
    }

    auto node = m_node[key];
    if (!node.IsDefined()) {
        if (required) {
            throw MissingRequiredPropertyConfigException(m_position, key);
        }
        return {};
    }

    return std::make_shared<const Node>(node);
}

std::shared_ptr<const Node> Node::mapAt(const char *key, bool required) const
{
    if (m_unusedPropertyTracker) {
        m_unusedPropertyTracker->registerPropertyAccess(key);
    }

    auto node = m_node[key];
    if (!node.IsDefined()) {
        if (required) {
            throw MissingRequiredPropertyConfigException(m_position, key);
        }
        return {};
    }

    if (!node.IsMap()) {
        Node tmp = node;
        throw InvalidNodeTypeConfigException(m_position, NodeType::Map, tmp.type());
    }
    return std::make_shared<const Node>(node);
}

Node Node::asSequence() const
{
    Node node = *this;
    node.m_allowImplicitConversionToSequence = true;
    return node;
}

Node Node::clone() const
{
    Node node = YAML::Clone(m_node);
    node.setPosition(m_position);
    return node;
}

Node Node::substringNode(const QString &substring) const
{
    try {
        Node node = Node::load(substring);
        node.setPosition(m_position);
        node.m_isSubstringNode = true;
        return node;
    } catch (const YamlCppConfigException &e) {
        auto copy = e;
        copy.setPosition(m_position);
        throw std::move(copy);
    }
}

std::map<std::shared_ptr<const Node>, std::shared_ptr<const Node>> Node::mapChildren() const
{
    std::map<std::shared_ptr<const Node>, std::shared_ptr<const Node>> result;
    for (auto it = m_node.begin(); it != m_node.end(); ++it) {
        result.emplace(std::make_shared<const Node>(it->first, this), std::make_shared<const Node>(it->second, this));
    }
    return result;
}

std::vector<std::shared_ptr<const Node>> Node::sequenceChildren(bool disableUnusedPropertyCheck) const
{
    if (!isSequence()) {
        if (m_allowImplicitConversionToSequence) {
            return {std::make_shared<const Node>(*this)};
        }
        throw InvalidNodeTypeConfigException(m_position, NodeType::Sequence, type());
    }

    std::vector<std::shared_ptr<const Node>> result;
    for (const auto &child : m_node) {
        auto node = std::make_shared<const Node>(child, this);
        if (disableUnusedPropertyCheck && node->m_unusedPropertyTracker) {
            node->m_unusedPropertyTracker->setEnabled(false);
        }

        result.push_back(std::move(node));
    }
    return result;
}

std::vector<std::shared_ptr<Node>> Node::sequenceChildren(bool disableUnusedPropertyCheck)
{
    if (!isSequence()) {
        if (m_allowImplicitConversionToSequence) {
            return {std::make_shared<Node>(*this)};
        }
        throw InvalidNodeTypeConfigException(m_position, NodeType::Sequence, type());
    }

    std::vector<std::shared_ptr<Node>> result;
    for (const auto &child : m_node) {
        auto node = std::make_shared<Node>(child, this);
        if (disableUnusedPropertyCheck && node->m_unusedPropertyTracker) {
            node->m_unusedPropertyTracker->setEnabled(false);
        }

        result.push_back(std::move(node));
    }
    return result;
}

NodeType Node::type() const
{
    if (isMap()) {
        return NodeType::Map;
    } else if (isScalar()) {
        return NodeType::Scalar;
    } else if (isSequence()) {
        return NodeType::Sequence;
    }
    return NodeType::Null;
}

bool Node::isMap() const
{
    return m_node.IsMap();
}

bool Node::isScalar() const
{
    return m_node.IsScalar();
}

bool Node::isSequence() const
{
    return m_node.IsSequence();
}

QString Node::tag() const
{
    return QString::fromStdString(m_node.Tag());
}

}