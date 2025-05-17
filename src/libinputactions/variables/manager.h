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

#include <map>
#include <memory>

#include <QString>

namespace libinputactions
{

class Variable;

template<typename T>
class VariableInfo
{
public:
    VariableInfo(const QString &name);

    const QString &name() const;

private:
    QString m_name;
};

template<typename T>
class VariableWrapper
{
public:
    VariableWrapper(Variable *variable);

    std::optional<T> get() const;
    void set(const std::optional<T> &value);

private:
    Variable *m_variable;
};

class BuiltinVariables
{
public:
    inline static const VariableInfo<qreal> Fingers{QStringLiteral("fingers")};
    inline static const VariableInfo<Qt::KeyboardModifiers> KeyboardModifiers{QStringLiteral("keyboard_modifiers")};
};

/**
 * Variables must be registered before loading the configuration file.
 */
class VariableManager
{
public:
    template<typename T>
    VariableWrapper<T> getVariable(const VariableInfo<T> &name);
    template<typename T>
    VariableWrapper<T> getVariable(const QString &name);
    Variable *getVariable(const QString &name);

    void registerVariable(const QString &name, std::unique_ptr<Variable> variable);
    template<typename T>
    void registerLocalVariable(const QString &name);
    template<typename T>
    void registerRemoteVariable(const QString &name, const std::function<void(std::optional<T> &value)> getter);

    QString expandString(const QString &s);

    static VariableManager *instance();

private:
    VariableManager();

    std::map<QString, std::unique_ptr<Variable>> m_variables;

    static std::unique_ptr<VariableManager> s_instance;
};

}