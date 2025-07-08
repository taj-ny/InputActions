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

#include <libinputactions/globals.h>
#include <libinputactions/variables/VariableManager.h>

#include <QProcess>

namespace libinputactions
{

template<typename T>
T fromString(const QString &s);

template<>
QString fromString(const QString &s)
{
    return s;
}

template<typename T>
Value<T>::Value(const T &value)
{
    m_value = value;
}

template<typename T>
Value<T>::Value(const std::function<T()> &getter)
{
    m_value = getter;
}

template<typename T>
Value<T> Value<T>::command(const Value<QString> &command)
{
    return Value<T>([command] {
        QProcess process;
        process.setProgram("/bin/sh");
        process.setArguments({"-c", command.get()});
        process.start();
        process.waitForFinished();
        return fromString<T>(process.readAllStandardOutput());
    });
}

template<typename T>
Value<T> Value<T>::variable(const QString &name)
{
    return Value<T>([name] {
        return VariableManager::instance()->getVariable<T>(name)->get().value();
    });
}

template<typename T>
Value<T>::Value(const Expression<T> &expression)
{
    m_value = [expression] {
        return expression.evaluate();
    };
}

template<typename T>
T Value<T>::get() const
{
    return std::visit(overloads {
        [](const T &value) -> T {
            return value;
        },
        [](const std::function<T()> &getter) {
            return getter();
        }
    }, m_value);
}

template class Value<QString>;

}