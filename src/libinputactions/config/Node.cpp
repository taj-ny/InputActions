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
#include "ConfigIssueManager.h"

namespace InputActions
{

Node::Node(YAML::Node node)
    : m_node(node)
    , m_line(node.Mark().line)
    , m_column(node.Mark().column)
{
}

Node::Node(QString text, const Node *parentNode)
    : m_node(YAML::Load(text.toStdString()))
    , m_line(parentNode->line())
    , m_column(parentNode->column())
{
}

Node::~Node()
{
    if (!isMap() || !m_mapAccessCheckEnabled) {
        return;
    }

    for (auto it = m_node.begin(); it != m_node.end(); ++it) {
        if (const auto key = QString::fromStdString(it->first.as<std::string>("")); !key.isEmpty()) {
            if (!m_accessedMapNodes.contains(key)) {
                Node tmp = m_node[key.toStdString().c_str()];
                g_configIssueManager->addIssue(&tmp,
                                               ConfigIssueSeverity::UnusedProperty,
                                               QString("Property '%1' does not exist or has no effect in this context.").arg(key));
            }
        }
    }
}

std::shared_ptr<const Node> Node::at(const char *key, bool required) const
{
    m_accessedMapNodes.insert(key);

    auto node = m_node[key];
    if (!node.IsDefined()) {
        if (required) {
            throw ConfigParserException(this, QString("Required property '%1' is missing.").arg(key));
        }
        return {};
    }

    return std::make_shared<const Node>(node);
}

std::shared_ptr<const Node> Node::mapAt(const char *key, bool required) const
{
    m_accessedMapNodes.insert(key);

    auto node = m_node[key];
    if (!node.IsDefined()) {
        if (required) {
            throw ConfigParserException(this, QString("Required property '%1' is missing.").arg(key));
        }
        return {};
    }

    if (!node.IsMap()) {
        Node tmp = node;
        throw ConfigParserException(&tmp, "Expected a map.");
    }
    return std::make_shared<const Node>(node);
}

Node Node::asSequence() const
{
    YAML::Node result(YAML::NodeType::Sequence);
    if (m_node.IsSequence()) {
        for (const auto &child : m_node) {
            result.push_back(child);
        }
    } else {
        result.push_back(m_node);
    }

    return result;
}

Node Node::clone() const
{
    Node newNode = YAML::Clone(m_node);
    newNode.setPosition(m_line, m_column);
    return newNode;
}

bool Node::isMap() const
{
    return m_node.IsMap();
}

std::map<std::shared_ptr<const Node>, std::shared_ptr<const Node>> Node::mapChildren() const
{
    std::map<std::shared_ptr<const Node>, std::shared_ptr<const Node>> result;
    for (auto it = m_node.begin(); it != m_node.end(); ++it) {
        result.emplace(std::make_shared<const Node>(it->first), std::make_shared<const Node>(it->second));
    }
    return result;
}

bool Node::isSequence() const
{
    return m_node.IsSequence();
}

std::vector<std::shared_ptr<const Node>> Node::sequenceChildren(bool disableMapAccessCheck) const
{
    if (!m_node.IsSequence()) {
        throw ConfigParserException(this, "Expected a list.");
    }

    std::vector<std::shared_ptr<const Node>> result;
    for (const auto &child : m_node) {
        auto node = std::make_shared<const Node>(child);
        if (disableMapAccessCheck) {
            node->disableMapAccessCheck();
        }

        result.push_back(std::move(node));
    }
    return result;
}

std::vector<std::shared_ptr<Node>> Node::sequenceChildren(bool disableMapAccessCheck)
{
    if (!m_node.IsSequence()) {
        throw ConfigParserException(this, "Expected a list.");
    }

    std::vector<std::shared_ptr<Node>> result;
    for (const auto &child : m_node) {
        auto node = std::make_shared<Node>(child);
        if (disableMapAccessCheck) {
            node->disableMapAccessCheck();
        }

        result.push_back(std::move(node));
    }
    return result;
}

Node::operator bool() const
{
    return static_cast<bool>(m_node);
}

}