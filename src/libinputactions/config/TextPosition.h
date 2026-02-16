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

#pragma once

#include <QDebug>
#include <QString>
#include <cstdint>

namespace InputActions
{

class TextPosition
{
public:
    TextPosition();
    TextPosition(int32_t line, int32_t column);

    int32_t line() const { return m_line; }
    void setLine(int32_t value) { m_line = value; }

    int32_t column() const { return m_column; }
    void setColumn(int32_t value) { m_column = value; }

    bool isValid() const;

    /**
     * "[line + 1]:[column + 1]: " if valid, empty string otherwise.
     */
    QString toString() const;

    bool operator==(const TextPosition &) const = default;

private:
    int32_t m_line;
    int32_t m_column;
};

inline QDebug operator<<(QDebug debug, const TextPosition &pos)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "TextPosition(" << pos.line() << ", " << pos.column() << ")";
    return debug;
}

}