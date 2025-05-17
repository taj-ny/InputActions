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

#include <optional>

namespace libinputactions
{

template<typename T>
class Range
{
public:
    Range() = default;
    Range(const T &minmax);
    Range(const std::optional<T> &min, const std::optional<T> &max);

    bool contains(const T &value) const;

    const std::optional<T> &min() const;
    const std::optional<T> &max() const;

    template<typename U>
    explicit operator Range<U>() const;

private:
    std::optional<T> m_min;
    std::optional<T> m_max;
};

}