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

#include "TextPosition.h"

namespace InputActions
{

TextPosition::TextPosition()
    : TextPosition(-1, -1)
{
}

TextPosition::TextPosition(int32_t line, int32_t column)
    : m_line(line)
    , m_column(column)
{
}

bool TextPosition::isValid() const
{
    return m_line >= 0 && m_column >= 0;
}

QString TextPosition::toString() const
{
    if (!isValid()) {
        return {};
    }

    return QString("%1:%2: ").arg(QString::number(m_line + 1), QString::number(m_column + 1));
}

}