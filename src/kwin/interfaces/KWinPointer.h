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

#include <libinputactions/interfaces/CursorShapeProvider.h>
#include <libinputactions/interfaces/PointerPositionGetter.h>
#include <libinputactions/interfaces/PointerPositionSetter.h>

namespace InputActions
{

class KWinPointer
    : public CursorShapeProvider
    , public PointerPositionGetter
    , public PointerPositionSetter
{
public:
    std::optional<CursorShape> cursorShape() override;

    std::optional<QPointF> globalPointerPosition() override;
    std::optional<QPointF> screenPointerPosition() override;

    void setGlobalPointerPosition(const QPointF &value) override;

private:
    std::optional<CursorShape> m_cachedShape;
};

}