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

#include "Value.h"
#include "InputActions.h"
#include <QProcess>
#include <libinputactions/globals.h>
#include <libinputactions/interfaces/CursorShapeProvider.h>
#include <libinputactions/variables/VariableManager.h>

namespace libinputactions
{

template<typename T>
T fromString(const QString &s)
{
    return T{};
}

template<>
QString fromString(const QString &s)
{
    return s;
}

template<typename T>
Value<T>::Value()
    : m_value(std::nullopt)
{
}

template<typename T>
Value<T>::Value(T value)
    : m_value(std::move(value))
{
}

template<typename T>
Value<T> Value<T>::command(Value<QString> command)
{
    auto value = Value<T>::function([command = std::move(command)]() -> std::optional<T> {
        const auto commandValue = command.get();
        if (!commandValue) {
            return {};
        }

        QProcess process;
        process.setProgram("/bin/sh");
        process.setArguments({"-c", commandValue.value()});
        g_variableManager->setProcessEnvironment(process);
        process.start();
        process.waitForFinished();
        return fromString<T>(process.readAllStandardOutput());
    });
    value.m_expensive = true;
    return value;
}

template<typename T>
Value<T> Value<T>::function(std::function<std::optional<T>()> function)
{
    auto value = Value<T>();
    value.m_value = std::move(function);
    return value;
}

template<typename T>
Value<T> Value<T>::variable(QString name)
{
    auto value = Value<T>::function([name = std::move(name)]() -> std::optional<T> {
        const auto *variable = g_variableManager->getVariable(name);
        if (!variable) {
            qCWarning(INPUTACTIONS).noquote() << QString("Failed to get value: variable %1 does not exist").arg(name);
            return {};
        }

        if constexpr (typeid(T) == typeid(QString)) {
            return variable->operations()->toString();
        }

        if (variable->type() != typeid(T)) {
            qCWarning(INPUTACTIONS).noquote()
                << QString("Failed to get value: variable %1 is of type %2, expected %3").arg(name, variable->type().name(), typeid(T).name());
            return {};
        }

        if (const auto variableValue = g_variableManager->getVariable<T>(name)->get()) {
            return variableValue;
        }

        qCWarning(INPUTACTIONS).noquote() << QString("Failed to get value: variable %1 is not set").arg(name);
        return {};
    });
    value.m_mainThreadOnly = true;
    return value;
}

template<typename T>
std::optional<T> Value<T>::get() const
{
    // clang-format off
    return std::visit(overloads {
        [](const std::optional<T> &value) {
            return value;
        },
        [this](const std::function<std::optional<T>()> &getter) {
            std::optional<T> value;
            if (m_mainThreadOnly) {
                InputActions::runOnMainThread([&value, getter]() {
                    value = getter();
                });
            } else {
                value = getter();
            }
            return value;
        }
    }, m_value);
    // clang-format on
}

template<typename T>
bool Value<T>::expensive() const
{
    return m_expensive;
}

template<typename T>
Value<T>::operator Value<std::any>() const
{
    return Value<std::any>::function([valueProvider = *this] -> std::any {
        if (const auto value = valueProvider.get()) {
            return value.value();
        }
        return {};
    });
}

template class Value<bool>;
template class Value<CursorShape>;
template class Value<Qt::KeyboardModifiers>;
template class Value<InputDeviceTypes>;
template class Value<qreal>;
template class Value<QPointF>;
template class Value<std::any>;
template class Value<QString>;

}