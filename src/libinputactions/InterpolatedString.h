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
#include <libinputactions/Value.h>
#include <set>

namespace libinputactions
{

/**
 * A string containing variable references ($variable) that will be replaced with the value of the variable.
 */
class InterpolatedString
{
public:
    InterpolatedString(QString string);

    operator Value<QString>() const;

private:
    QString evaluate() const;

    QString m_string;
    /**
     * Referenced variables.
     */
    std::set<QString> m_variables;
};

}