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

#include "utils/HyprlandFunctionHook.h"
#include <libinputactions/interfaces/CursorShapeProvider.h>
#include <libinputactions/interfaces/PointerPositionGetter.h>
#include <libinputactions/interfaces/PointerPositionSetter.h>

namespace InputActions
{

class HyprlandPointer
    : public CursorShapeProvider
    , public PointerPositionGetter
    , public PointerPositionSetter
{
public:
    HyprlandPointer(void *handle);

    std::optional<CursorShape> cursorShape() override;

    std::optional<QPointF> globalPointerPosition() override;
    std::optional<QPointF> screenPointerPosition() override;

    void setGlobalPointerPosition(const QPointF &value) override;

private:
    static void setCursorFromNameHook(void *thisPtr, const std::string &name);

    HyprlandFunctionHook<&setCursorFromNameHook> m_setCursorFromNameHook;
    QString m_currentCursorShape;
};

}