/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2025 Marcin Woźniak

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


#include <libinputactions/triggers/motion.h>

#include <any>
#include <chrono>
#include <cstdint>
#include <map>
#include <typeindex>

#include <QPointF>
#include <QString>

namespace libinputactions
{

template <typename T>
class VariableInfo
{
public:
    VariableInfo(const QString &name);

    const QString &name() const;

private:
    QString m_name;
};

enum class ComparisonOperator
{
    EqualTo,
    NotEqualTo,

    // List (right only)
    OneOf,

    // Number
    GreaterThan,
    GreaterThanOrEqual,
    LessThan,
    LessThanOrEqual,
    Between,

    // String
    Contains,
    Regex
};

/**
 * Supports the following types: bool, qreal, QString
 */
class Variable
{
public:
    Variable(const std::type_index &type);
    virtual ~Variable() = default;

    virtual std::any getAny() const;
    virtual void setAny(const std::any &value);

    /**
     * @param values Some operators may require multiple values.
     */
    bool compare(const std::vector<std::any> &values, const ComparisonOperator &comparisonOperator) const;

    const std::type_index &type() const;

protected:
    virtual bool compare(const std::any &left, const std::any &right, const ComparisonOperator &comparisonOperator) const;

private:
    std::type_index m_type;
};

template <typename T>
class TypedVariable : public Variable
{
public:
    TypedVariable();

    virtual std::optional<T> get() const;
    virtual void set(const std::optional<T> &value);

    std::any getAny() const override;
    void setAny(const std::any &value) override;

    static bool compare(const T &left, const T &right, const ComparisonOperator &comparisonOperator);

protected:
    bool compare(const std::any &left, const std::any &right, const ComparisonOperator &comparisonOperator) const override;
};

/**
 * A locally stored variable with quick access.
 */
template <typename T>
class LocalVariable : public TypedVariable<T>
{
public:
    LocalVariable() = default;

    std::optional<T> get() const override;
    void set(const std::optional<T> &value) override;

private:
    std::optional<T> m_value;
};

/**
 * A remotely stored, optionally cached variable that may be expensive to access.
 */
template <typename T>
class RemoteVariable : public TypedVariable<T>
{
public:
    /**
     * @param cacheExpiration How long to cache the value for, 0 - no caching
     */
    explicit RemoteVariable(const std::function<void(std::optional<T> &value)> &getter);

    std::optional<T> get() const override;

private:
    std::function<void(std::optional<T> &value)> m_getter;
};
 
class BuiltinVariables
{
public:
    inline static const VariableInfo<qreal> Fingers{QStringLiteral("fingers")};
    inline static const VariableInfo<Qt::KeyboardModifiers> KeyboardModifiers{QStringLiteral("keyboard_modifiers")};
    inline static const VariableInfo<Qt::MouseButtons> MouseButtons{QStringLiteral("mouse_buttons")};
};

class VariableManager
{
public:
    template <typename T>
    TypedVariable<T> *getVariable(const VariableInfo<T> &name);
    template <typename T>
    TypedVariable<T> *getVariable(const QString &name);
    Variable *getVariable(const QString &name);

    void registerVariable(const QString &name, std::unique_ptr<Variable> variable);
    template <typename T>
    void registerLocalVariable(const QString &name);
    template <typename T>
    void registerRemoteVariable(const QString &name, const std::function<void(std::optional<T> &value)> getter);

    QString expandString(const QString &s) const;

    static VariableManager *instance();

private:
    VariableManager();

    std::map<QString, std::unique_ptr<Variable>> m_variables;

    static std::unique_ptr<VariableManager> s_instance;
};

}