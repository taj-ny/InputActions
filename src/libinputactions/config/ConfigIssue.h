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
#include <QString>
#include <libinputactions/utils/Copyable.h>

namespace InputActions
{

class Node;
enum class NodeType;

enum class ConfigIssueSeverity
{
    Warning,
    Error
};

class ConfigIssue : public virtual CopyableBase<ConfigIssue>
{
public:
    virtual ~ConfigIssue() = default;

    const TextPosition &position() const { return m_position; }
    void setPosition(TextPosition value) { m_position = value; }

    QString colorAnsiSequence() const;

    /**
     * Format:
     *   - with position: [line]:[column]: [severity]: [message]
     *   - without position: [severity]: [message]
     */
    QString toString(bool colors = true) const;

    virtual ConfigIssueSeverity severity() const { return ConfigIssueSeverity::Warning; }
    virtual QString suppressKey() const { return ""; }
    virtual QString message() const = 0;

    bool operator==(const ConfigIssue &) const;

protected:
    ConfigIssue(const Node *node);
    ConfigIssue(TextPosition position);

private:
    bool m_isNodeSubstring{};
    QString m_substringNodeValue;
    TextPosition m_position;
};

class ConfigException
    : public ConfigIssue
    , public std::exception
{
public:
    ConfigException(const Node *node);
    ConfigException(TextPosition position);

    const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override;

    ConfigIssueSeverity severity() const final { return ConfigIssueSeverity::Error; }

private:
    mutable std::string m_what;
};

enum class DeprecatedFeature
{
    LegacyConditions,
    TouchpadDevicesNode,
    TriggerHandlerSettings,
    TriggerKeyboardModifiers,
};

class DeprecatedFeatureConfigIssue
    : public ConfigIssue
    , public virtual Copyable<DeprecatedFeatureConfigIssue, ConfigIssue>
{
public:
    DeprecatedFeatureConfigIssue(const Node *node, DeprecatedFeature feature);

    DeprecatedFeature feature() const { return m_feature; }

    QString message() const override;

private:
    DeprecatedFeature m_feature;
};

class UnusedPropertyConfigIssue
    : public ConfigIssue
    , public virtual Copyable<UnusedPropertyConfigIssue, ConfigIssue>
{
public:
    UnusedPropertyConfigIssue(const Node *node, QString property);

    const QString &property() const { return m_property; }

    QString message() const override;

private:
    QString m_property;
};

class DuplicateSetItemConfigException
    : public ConfigException
    , public virtual Copyable<DuplicateSetItemConfigException, ConfigIssue>
{
public:
    DuplicateSetItemConfigException(const Node *node, size_t index);

    size_t index() const { return m_index; }

    QString message() const override;

private:
    size_t m_index;
};

class InvalidNodeTypeConfigException
    : public ConfigException
    , public virtual Copyable<InvalidNodeTypeConfigException, ConfigIssue>
{
public:
    InvalidNodeTypeConfigException(const Node *node, NodeType expected);

    NodeType expected() const { return m_expected; }
    NodeType actual() const { return m_actual; }

    QString message() const override;

private:
    NodeType m_expected;
    NodeType m_actual;
};

class InvalidValueConfigException
    : public ConfigException
    , public virtual Copyable<InvalidValueConfigException, ConfigIssue>
{
public:
    InvalidValueConfigException(const Node *node, QString message);

    QString message() const override { return m_message; }

private:
    QString m_message;
};

/**
 * Value is valid but not in the current context.
 */
class InvalidValueContextConfigException
    : public ConfigException
    , public virtual Copyable<InvalidValueContextConfigException, ConfigIssue>
{
public:
    InvalidValueContextConfigException(const Node *node, QString message);

    QString message() const override { return m_message; }

private:
    QString m_message;
};

class InvalidVariableConfigException
    : public ConfigException
    , public virtual Copyable<InvalidVariableConfigException, ConfigIssue>
{
public:
    InvalidVariableConfigException(const Node *node, QString variableName);

    const QString &variableName() const { return m_variableName; }

    QString message() const override;

private:
    QString m_variableName;
};

class MissingRequiredPropertyConfigException
    : public ConfigException
    , public virtual Copyable<MissingRequiredPropertyConfigException, ConfigIssue>
{
public:
    MissingRequiredPropertyConfigException(const Node *node, QString property);

    const QString &property() const { return m_property; }

    QString message() const override;

private:
    QString m_property;
};

class YamlCppConfigException
    : public ConfigException
    , public virtual Copyable<YamlCppConfigException, ConfigIssue>
{
public:
    YamlCppConfigException(TextPosition position, QString message);

    QString message() const override { return m_message; }

private:
    QString m_message;
};

}