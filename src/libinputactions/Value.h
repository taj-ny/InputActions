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

#include "Expression.h"
#include <QString>
#include <functional>
#include <variant>

namespace libinputactions
{

template<typename T>
class Value
{
public:
    Value(T value = {});
    Value(std::function<T()> getter);
    Value(Expression<T> expression);

    static Value<T> command(Value<QString> command);
    static Value<T> variable(QString name);

    /**
     * Safe to call from other threads, will dispatch to main and block if required.
     */
    T get() const;
    /**
     * Whether evaluating the value may be expensive.
     */
    bool expensive() const;

private:
    std::variant<T, std::function<T()>> m_value;
    /**
     * Whether the value can only be evaluated on the main thread.
     */
    bool m_mainThreadOnly{};
    bool m_expensive{};
};

}