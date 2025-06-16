/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2025 Marcin Woźniak

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

#include <libinputactions/globals.h>

#include <QPointF>

namespace libinputactions
{

class PointerPositionGetter
{
    INPUTACTIONS_DECLARE_SINGLETON(PointerPositionGetter)

public:
    PointerPositionGetter() = default;
    virtual ~PointerPositionGetter() = default;

    /**
     * @return Global position in pixels or std::nullopt if not available.
     */
    virtual std::optional<QPointF> globalPointerPosition();
    /**
     * @return Position relative to the current screen as a percentage ranging from (0.0,0.0) to (1.0,1.0), or std::nullopt if not available.
     */
    virtual std::optional<QPointF> screenPointerPosition();
};

}