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
#include "TextPosition.h"
#include <QLoggingCategory>
#include <libinputactions/helpers/QString.h>

namespace InputActions
{

Node::Node(Private, NodeType type)
    : m_type(type)
    , m_position(-1, -1)
{
}

Node::Node(Private, YAML::Node node)
    : m_node(node)
    , m_tag(QString::fromStdString(node.Tag()))
    , m_position(node.Mark().line, node.Mark().column)
{
    if (node.IsMap()) {
        m_type = NodeType::Map;
        for (auto it = m_node.begin(); it != m_node.end(); ++it) {
            m_mapItems.emplace(Node::create(it->first), Node::create(it->second));
        }
    } else if (node.IsSequence()) {
        m_type = NodeType::Sequence;
        for (const auto &item : m_node) {
            m_sequenceItems.push_back(Node::create(item));
        }
    } else if (node.IsScalar()) {
        m_type = NodeType::Scalar;
    } else {
        m_type = NodeType::Null;
        if (m_position == TextPosition(-1, -1)) {
            m_position = {0, 0};
        }
    }
}

Node::~Node() = default;

std::shared_ptr<Node> Node::create(NodeType type)
{
    return std::make_shared<Node>(Private(), type);
}

std::shared_ptr<Node> Node::create(YAML::Node node)
{
    return std::make_shared<Node>(Private(), std::move(node));
}

std::shared_ptr<Node> Node::create(const QString &s)
{
    try {
        return Node::create(YAML::Load(s.toStdString()));
    } catch (const YAML::Exception &e) {
        throw YamlCppConfigException({e.mark.line, e.mark.column}, e.what());
    }
}

void Node::addUnusedMapPropertyIssues() const
{
    if (isMap()) {
        for (const auto &[key, node] : m_mapItems) {
            if (!node->m_used) {
                g_configIssueManager->addIssue(UnusedPropertyConfigIssue(key.get(), key->as<QString>()));
            }
            node->addUnusedMapPropertyIssues();
        }
    } else if (isSequence()) {
        for (const auto &node : m_sequenceItems) {
            node->addUnusedMapPropertyIssues();
        }
    }
}

const Node *Node::at(const char *key, bool required) const
{
    const auto items = mapItems();
    if (!items.contains(key)) {
        if (required) {
            throw MissingRequiredPropertyConfigException(this, key);
        }
        return {};
    }

    const auto *node = items.at(key);
    node->m_used = true;
    return node;
}

const Node *Node::mapAt(const char *key, bool required) const
{
    const auto items = mapItems();
    if (!items.contains(key)) {
        if (required) {
            throw MissingRequiredPropertyConfigException(this, key);
        }
        return {};
    }

    const auto *node = items.at(key);
    if (!node->isMap()) {
        throw InvalidNodeTypeConfigException(node, NodeType::Map);
    }
    node->m_used = true;
    return node;
}

std::shared_ptr<Node> Node::substringNode(const QString &substring) const
{
    try {
        // Try to preserve the position
        QString raw;
        for (auto i = 0; i < m_position.line(); i++) {
            raw += '\n';
        }
        // TODO: This will sometimes be inaccurate, make substringNode accept a start index and length instead
        const auto index = std::max(static_cast<qsizetype>(0), as<QString>().indexOf(substring, 0, Qt::CaseInsensitive));
        raw += QStringHelpers::indented(substring, m_position.column() + index);

        auto node = Node::create(raw);
        node->m_substringValue = substring;
        if (node->m_position == TextPosition(0, 0)) {
            node->m_position = m_position;
        }

        // Fine for now, no maps or nested sequence substrings
        for (const auto &item : node->m_sequenceItems) {
            item->m_substringValue = substring;
            if (item->m_position == TextPosition(0, 0)) {
                item->m_position = m_position;
            }
        }

        return node;
    } catch (const YamlCppConfigException &e) {
        auto copy = e;
        copy.setPosition(m_position);
        throw std::move(copy);
    }
}

std::shared_ptr<Node> Node::substringNodeQuoted(const QString &substring) const
{
    try {
        const auto index = std::max(static_cast<qsizetype>(0), as<QString>().indexOf(substring, 0, Qt::CaseInsensitive));

        auto node = Node::create(QStringHelpers::quoted(substring));
        node->setPosition({m_position.line(), m_position.column() + static_cast<int32_t>(index)});
        node->m_substringValue = substring;

        return node;
    } catch (const YamlCppConfigException &e) {
        auto copy = e;
        copy.setPosition(m_position);
        throw std::move(copy);
    }
}

std::map<QString, const Node *> Node::mapItems() const
{
    if (!isMap()) {
        throw InvalidNodeTypeConfigException(this, NodeType::Map);
    }

    std::map<QString, const Node *> result;
    for (const auto &[key, value] : m_mapItems) {
        result.emplace(key->as<QString>(), value.get());
    }
    return result;
}

std::map<const Node *, const Node *> Node::mapItemsRawKeys() const
{
    if (!isMap()) {
        throw InvalidNodeTypeConfigException(this, NodeType::Map);
    }

    std::map<const Node *, const Node *> result;
    for (const auto &[key, value] : m_mapItems) {
        result.emplace(key.get(), value.get());
    }
    return result;
}

std::vector<const Node *> Node::sequenceItems() const
{
    if (!isSequence()) {
        if (m_allowImplicitConversionToSequence) {
            return {this};
        }
        throw InvalidNodeTypeConfigException(this, NodeType::Sequence);
    }

    std::vector<const Node *> result;
    for (const auto &item : m_sequenceItems) {
        result.push_back(item.get());
    }
    return result;
}

void Node::addMapItem(std::shared_ptr<const Node> key, std::shared_ptr<const Node> value)
{
    m_mapItems[key] = std::move(value);
}

bool Node::isNull() const
{
    return m_type == NodeType::Null;
}

bool Node::isMap() const
{
    return m_type == NodeType::Map;
}

bool Node::isScalar() const
{
    return m_type == NodeType::Scalar;
}

bool Node::isSequence() const
{
    return m_type == NodeType::Sequence;
}

bool Node::isSubstring() const
{
    return m_substringValue.has_value();
}

QString Node::substring() const
{
    return m_substringValue.value_or(QString());
}

}