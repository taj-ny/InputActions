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

#include "KWinPointer.h"
#include "KWinInputEmitter.h"
#include "core/output.h"
#include "cursor.h"
#include "cursorsource.h"
#include "pointer_input.h"
#include "utils.h"
#include "workspace.h"
#include <libinputactions/input/backends/InputBackend.h>

using namespace InputActions;

std::optional<CursorShape> KWinPointer::cursorShape()
{
    auto *cursors = KWin::Cursors::self();
    auto *cursor = cursors->currentCursor();
    if (!cursor) {
        return {};
    }

    auto *shapeSource = dynamic_cast<KWin::ShapeCursorSource *>(cursor->source());
    if (!shapeSource) {
        if (cursors->isCursorHidden() && m_cachedShape) {
            // The cursor may be hidden after typing text
            return m_cachedShape;
        }
        return {};
    }

    const QString shapeName(shapeSource->shape().replace('-', '_'));
    if (CURSOR_SHAPES.contains(shapeName)) {
        m_cachedShape = CURSOR_SHAPES.at(shapeName);
        return m_cachedShape;
    }
    return {};
}

std::optional<QPointF> KWinPointer::globalPointerPosition()
{
    return KWin::input()->pointer()->pos();
}

std::optional<QPointF> KWinPointer::screenPointerPosition()
{
    QPointF position;
    const auto rawPosition = KWin::input()->pointer()->pos();
    for (const auto &output : KWin::workspace()->outputs()) {
        const auto geometry = output->geometryF();
        if (!geometry.contains(rawPosition)) {
            continue;
        }

        const auto translatedPosition = rawPosition - geometry.topLeft();
        position.setX(translatedPosition.x() / geometry.width());
        position.setY(translatedPosition.y() / geometry.height());
    }
    return position;
}

void KWinPointer::setGlobalPointerPosition(const QPointF &position)
{
    auto *device = static_cast<KWinInputEmitter *>(g_inputEmitter.get())->device();
    g_inputBackend->setIgnoreEvents(true);
    Q_EMIT device->pointerMotionAbsolute(position, timestamp(), device);
    Q_EMIT device->pointerFrame(device);
    g_inputBackend->setIgnoreEvents(false);
}