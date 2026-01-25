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
#include <libinputactions/utils/Copyable.h>
#include <QString>

namespace InputActions
{

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
    QString toString() const;

    virtual ConfigIssueSeverity severity() const { return ConfigIssueSeverity::Warning; }
    virtual QString suppressKey() const { return ""; }
    virtual QString message() const = 0;

    bool operator==(const ConfigIssue &) const;

protected:
    ConfigIssue(TextPosition position);

private:
    TextPosition m_position;
};

class ConfigException
    : public ConfigIssue
    , public std::exception
{
public:
    ConfigException(TextPosition position);

    ConfigIssueSeverity severity() const final { return ConfigIssueSeverity::Error; }
};

enum class DeprecatedFeature
{
    LegacyConditions,
    TouchpadDevicesNode,
};

class DeprecatedFeatureConfigIssue
    : public ConfigIssue
    , public virtual Copyable<DeprecatedFeatureConfigIssue, ConfigIssue>
{
public:
    DeprecatedFeatureConfigIssue(TextPosition position, DeprecatedFeature feature);

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
    UnusedPropertyConfigIssue(TextPosition position, QString property);

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
    DuplicateSetItemConfigException(TextPosition position, size_t index);

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
    InvalidNodeTypeConfigException(TextPosition position, NodeType expected, NodeType actual);

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
    InvalidValueConfigException(TextPosition position, QString message);

    QString message() const override { return m_message; }

private:
    QString m_message;
};

class InvalidVariableConfigException
    : public ConfigException
    , public virtual Copyable<InvalidVariableConfigException, ConfigIssue>
{
public:
    InvalidVariableConfigException(TextPosition position, QString variableName);

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
    MissingRequiredPropertyConfigException(TextPosition position, QString property);

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