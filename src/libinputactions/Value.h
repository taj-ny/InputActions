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

#include <QString>
#include <any>
#include <functional>
#include <variant>

namespace InputActions
{

template<typename T>
class Value
{
public:
    Value();
    /**
     * Constructs a Value that always returns the specified value.
     */
    Value(T value);

    /**
     * Constructs a Value that returns the standard output of the specified command.
     */
    static Value<T> command(Value<QString> command);

    /**
     * Constructs a Value that returns the value returned by the specified function.
     */
    static Value<T> function(std::function<std::optional<T>()> function);

    /*
     * Constructs a Value that returns the value of the specified variable.
     */
    static Value<T> variable(QString name);

    /**
     * Safe to call from other threads, will dispatch to main and block if required.
     */
    std::optional<T> get() const;
    /**
     * Whether evaluating the value may be expensive.
     */
    bool expensive() const;

    operator Value<std::any>() const;

private:
    std::variant<std::optional<T>, std::function<std::optional<T>()>> m_value;
    /**
     * Whether the value can only be evaluated on the main thread.
     */
    bool m_mainThreadOnly{};
    bool m_expensive{};
};

}