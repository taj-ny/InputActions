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
#include "ConfigIssueManager.h"
#include "Node.h"
#include "../../common/ansi-escape-codes.h"
#include <QRegularExpression>

namespace InputActions
{

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
    }
}

QString ConfigIssue::toString() const
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
    severityString = AnsiEscapeCode::Color::Bold + colorAnsiSequence() + severityString + AnsiEscapeCode::Color::Reset;

    return QString("%1%2: %3").arg(m_position.toString(), severityString, message());
}

bool ConfigIssue::operator==(const ConfigIssue &other) const
{
    return m_position == other.m_position && severity() == other.severity();
}

ConfigException::ConfigException(TextPosition position)
    : ConfigIssue(position)
{
}

DeprecatedFeatureConfigIssue::DeprecatedFeatureConfigIssue(TextPosition position, DeprecatedFeature feature)
    : ConfigIssue(position)
    , m_feature(feature)
{
}

QString DeprecatedFeatureConfigIssue::message() const
{
    switch (m_feature) {
        case DeprecatedFeature::LegacyConditions:
            return "This method of defining conditions is deprecated";
        case DeprecatedFeature::TouchpadDevicesNode:
            return "This method of defining device properties is deprecated. Use device_rules instead.";
    }
}

DuplicateSetItemConfigException::DuplicateSetItemConfigException(TextPosition position, size_t index)
    : ConfigException(position)
    , m_index(index)
{
}

InvalidNodeTypeConfigException::InvalidNodeTypeConfigException(TextPosition position, NodeType expected, NodeType actual)
    : ConfigException(position)
    , m_expected(expected)
    , m_actual(actual)
{
}

QString InvalidNodeTypeConfigException::message() const
{
    static const std::unordered_map<NodeType, QString> enumStringValues{
        {NodeType::Scalar, "a scalar (neither a list nor a map)"},
        {NodeType::Sequence, "a list"},
        {NodeType::Map, "a map"},
        {NodeType::Null, "an empty value"},
    };

    return QString("Expected %1, but got %2.").arg(enumStringValues.at(m_expected), enumStringValues.at(m_actual));
}

InvalidValueConfigException::InvalidValueConfigException(TextPosition position, QString message)
    : ConfigException(position)
    , m_message(std::move(message))
{
}

InvalidVariableConfigException::InvalidVariableConfigException(TextPosition position, QString variableName)
    : ConfigException(position)
    , m_variableName(std::move(variableName))
{
}

QString InvalidVariableConfigException::message() const
{
    return QString("Variable '%1' does not exist.").arg(m_variableName);
}

QString DuplicateSetItemConfigException::message() const
{
    return QString("This list may only contain unique items, but the item at position %1 (starting from 1) has already been specified before.").arg(QString::number(m_index + 1));
}

UnusedPropertyConfigIssue::UnusedPropertyConfigIssue(TextPosition position, QString property)
    : ConfigIssue(position)
    , m_property(std::move(property))
{
}

QString UnusedPropertyConfigIssue::message() const
{
    return QString("Property '%1' does not exist or has no effect in this context. If this is a trigger group, at least one trigger did not use it.").arg(m_property);
}

MissingRequiredPropertyConfigException::MissingRequiredPropertyConfigException(TextPosition position, QString property)
    : ConfigException(position)
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
    m_message += ". This is likely a syntax error.";
}

}