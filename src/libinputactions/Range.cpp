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

#include "Range.h"

#include <limits>

#include <QPointF>

namespace libinputactions
{

template<typename T>
Range<T>::Range(const T &minmax)
    : Range(minmax, minmax)
{
}

template<typename T>
Range<T>::Range(const std::optional<T> &min, const std::optional<T> &max)
    : m_min(min)
    , m_max(max)
{
}

template<typename T>
bool Range<T>::contains(const T &value) const
{
    return (!m_min || value >= m_min) && (!m_max || value <= m_max);
}

template<>
bool Range<QPointF>::contains(const QPointF &value) const
{
    return (!m_min || (value.x() >= m_min->x() && value.y() >= m_min->y()))
        && (!m_max || (value.x() <= m_max->x() && value.y() <= m_max->y()));
}

template<typename T>
const std::optional<T> &Range<T>::min() const
{
    return m_min;
}

template<typename T>
const std::optional<T> &Range<T>::max() const
{
    return m_max;
}

template<typename T>
template<typename U>
Range<T>::operator Range<U>() const
{
    return Range<U>(static_cast<std::optional<U>>(m_min), static_cast<std::optional<U>>(m_max));
}

template class Range<uint8_t>;
template class Range<uint32_t>;
template class Range<qreal>;
template class Range<QPointF>;

}