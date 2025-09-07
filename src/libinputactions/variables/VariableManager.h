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

#include "LocalVariable.h"
#include "RemoteVariable.h"
#include "VariableWrapper.h"
#include <QPointF>
#include <QString>
#include <map>
#include <memory>

Q_DECLARE_LOGGING_CATEGORY(INPUTACTIONS_VARIABLE_MANAGER)

namespace libinputactions
{

const static uint8_t s_fingerVariableCount = 5;

class Variable;

template<typename T>
struct VariableInfo
{
    QString name;

    operator QString() const
    {
        return name;
    }
};

struct BuiltinVariables
{
    inline static const VariableInfo<QString> DeviceName{QStringLiteral("device_name")};
    inline static const VariableInfo<qreal> Fingers{QStringLiteral("fingers")};
    inline static const VariableInfo<Qt::KeyboardModifiers> KeyboardModifiers{QStringLiteral("keyboard_modifiers")};
    inline static const VariableInfo<QString> LastTriggerId{QStringLiteral("last_trigger_id")};
    inline static const VariableInfo<QPointF> ThumbInitialPositionPercentage{QStringLiteral("thumb_initial_position_percentage")};
    inline static const VariableInfo<QPointF> ThumbPositionPercentage{QStringLiteral("thumb_position_percentage")};
    inline static const VariableInfo<bool> ThumbPresent{QStringLiteral("thumb_present")};
};

/**
 * Variables must be registered before loading the configuration file.
 */
class VariableManager
{
public:
    VariableManager();
    ~VariableManager();

    template<typename T>
    std::optional<VariableWrapper<T>> getVariable(const VariableInfo<T> &variable)
    {
        return getVariable<T>(variable.name);
    }

    /**
     * @return A statically-typed wrapper for the specified variable, nullptr if not found or type doesn't match.
     */
    template<typename T>
    std::optional<VariableWrapper<T>> getVariable(const QString &name)
    {
        auto *variable = getVariable(name);
        if (!variable) {
            return {};
        } else if (variable->type() != typeid(T)) {
            qCWarning(INPUTACTIONS_VARIABLE_MANAGER).noquote()
                << QString("VariableManager::getVariable<T> called with the wrong type (variable: %1, type: %2").arg(variable->type().name(), typeid(T).name());
            return {};
        }

        return VariableWrapper<T>(getVariable(name));
    }

    bool hasVariable(const QString &name);
    /**
     * @return The variable with the specified name or nullptr if not found.
     */
    Variable *getVariable(const QString &name);

    void registerVariable(const QString &name, std::unique_ptr<Variable> variable, bool hidden = false);
    template<typename T>
    void registerLocalVariable(const QString &name)
    {
        registerVariable(name, std::make_unique<LocalVariable>(typeid(T)));
    }
    template<typename T>
    void registerLocalVariable(const VariableInfo<T> &variable)
    {
        registerLocalVariable<T>(variable.name);
    }
    template<typename T>
    void registerRemoteVariable(const QString &name, const std::function<void(std::optional<T> &value)> getter, bool hidden = false)
    {
        const std::function<void(std::any & value)> anyGetter = [getter](std::any &value) {
            std::optional<T> optValue;
            getter(optValue);
            if (optValue.has_value()) {
                value = optValue.value();
            }
        };
        registerVariable(name, std::make_unique<RemoteVariable>(typeid(T), anyGetter), hidden);
    }

    std::map<QString, const Variable *> variables() const;

private:
    std::map<QString, std::unique_ptr<Variable>> m_variables;
};

inline auto g_variableManager = std::make_shared<VariableManager>();

}