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

#include "ConfigIssue.h"
#include "../../common/ansi-escape-codes.h"
#include "ConfigIssueManager.h"
#include "Node.h"
#include <QRegularExpression>

namespace InputActions
{

ConfigIssue::ConfigIssue(const Node *node)
    : m_isNodeSubstring(node->isSubstring())
    , m_substringNodeValue(node->substring())
    , m_position(node->position())
{
}

ConfigIssue::ConfigIssue(TextPosition position)
    : m_position(position)
{
}

QString ConfigIssue::colorAnsiSequence() const
{
    switch (severity()) {
        case ConfigIssueSeverity::Error:
            return AnsiEscapeCode::Color::Red;
        case ConfigIssueSeverity::Warning:
            return AnsiEscapeCode::Color::Yellow;
        default:
            return {};
    }
}

QString ConfigIssue::toString(bool colors) const
{
    QString severityString;
    switch (severity()) {
        case ConfigIssueSeverity::Warning:
            severityString = "warning";
            break;
        case ConfigIssueSeverity::Error:
            severityString = "error";
            break;
    }
    if (colors) {
        severityString = AnsiEscapeCode::Color::Bold + colorAnsiSequence() + severityString + AnsiEscapeCode::Color::Reset;
    }

    auto text = message();
    if (m_isNodeSubstring) {
        text = QString("While parsing substring '%1': %2%3 The provided position of this issue is an approximation and may be incorrect.")
                   .arg(m_substringNodeValue, text.first(1).toLower(), text.mid(1));
    }

    return QString("%1%2: %3").arg(m_position.toString(), severityString, text);
}

bool ConfigIssue::operator==(const ConfigIssue &other) const
{
    return m_position == other.m_position && severity() == other.severity();
}

ConfigException::ConfigException(const Node *node)
    : ConfigIssue(node)
{
}

ConfigException::ConfigException(TextPosition position)
    : ConfigIssue(position)
{
}

const char *ConfigException::what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW
{
    if (m_what.empty()) {
        m_what = toString().toStdString();
    }
    return m_what.c_str();
}

DeprecatedFeatureConfigIssue::DeprecatedFeatureConfigIssue(const Node *node, DeprecatedFeature feature)
    : ConfigIssue(node)
    , m_feature(feature)
{
}

QString DeprecatedFeatureConfigIssue::message() const
{
    switch (m_feature) {
        case DeprecatedFeature::LegacyConditions:
            return "This method of defining conditions is deprecated.";
        case DeprecatedFeature::TouchpadDevicesNode:
            return "This method of defining device properties is deprecated, use device rules instead.";
        case DeprecatedFeature::TriggerHandlerSettings:
            return "This property has been moved to device properties and can be configured through device rules.";
        case DeprecatedFeature::TriggerKeyboardModifiers:
            return "This method of specifying trigger keyboard modifiers is deprecated, use a '$keyboard_modifiers' variable condition instead.";
        default:
            return {};
    }
}

DuplicateSetItemConfigException::DuplicateSetItemConfigException(const Node *node, size_t index)
    : ConfigException(node)
    , m_index(index)
{
}

InvalidNodeTypeConfigException::InvalidNodeTypeConfigException(const Node *node, NodeType expected)
    : ConfigException(node)
    , m_expected(expected)
    , m_actual(node->type())
{
}

QString InvalidNodeTypeConfigException::message() const
{
    static const std::unordered_map<NodeType, QString> enumStringValues{
        {NodeType::Scalar, "a scalar"},
        {NodeType::Sequence, "a list"},
        {NodeType::Map, "a map"},
        {NodeType::Null, "null"},
    };

    return QString("Expected %1, but got %2.").arg(enumStringValues.at(m_expected), enumStringValues.at(m_actual));
}

InvalidValueConfigException::InvalidValueConfigException(const Node *node, QString message)
    : ConfigException(node)
    , m_message(std::move(message))
{
}

InvalidValueContextConfigException::InvalidValueContextConfigException(const Node *node, QString message)
    : ConfigException(node)
    , m_message(std::move(message))
{
}

InvalidVariableConfigException::InvalidVariableConfigException(const Node *node, QString variableName)
    : ConfigException(node)
    , m_variableName(std::move(variableName))
{
}

QString InvalidVariableConfigException::message() const
{
    return QString("Variable '%1' does not exist.").arg(m_variableName);
}

QString DuplicateSetItemConfigException::message() const
{
    return QString("This list may only contain unique items, but the item at position %1 (starting from 1) has already been specified before.")
        .arg(QString::number(m_index + 1));
}

UnusedPropertyConfigIssue::UnusedPropertyConfigIssue(const Node *node, QString property)
    : ConfigIssue(node)
    , m_property(std::move(property))
{
}

QString UnusedPropertyConfigIssue::message() const
{
    return QString("Property '%1' does not exist or has no effect in this context.").arg(m_property);
}

MissingRequiredPropertyConfigException::MissingRequiredPropertyConfigException(const Node *node, QString property)
    : ConfigException(node)
    , m_property(std::move(property))
{
}

QString MissingRequiredPropertyConfigException::message() const
{
    return QString("Required property '%1' was not specified.").arg(m_property);
}

YamlCppConfigException::YamlCppConfigException(TextPosition position, QString message)
    : ConfigException(position)
    , m_message(std::move(message))
{
    m_message.replace(QRegularExpression("yaml-cpp: error at line \\d+, column \\d+: "), "");
    m_message[0] = m_message[0].toUpper();
    m_message += ".";
}

}