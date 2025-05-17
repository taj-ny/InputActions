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

#include "expression.h"

#include <libinputactions/globals.h>
#include <libinputactions/variables/manager.h>

#include <QString>

namespace libinputactions
{

template<typename T>
Expression<T>::Expression(const QString &expression)
{
    m_expression = expression;
}

template<>
QString Expression<QString>::evaluate() const
{
    return VariableManager::instance()->expandString(m_expression);
}

template class Expression<QString>;

}