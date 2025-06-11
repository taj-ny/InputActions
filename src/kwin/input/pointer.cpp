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

#include "pointer.h"
#include "emitter.h"
#include "utils.h"

#include "cursor.h"
#include "cursorsource.h"
#include "core/output.h"
#include "pointer_input.h"
#include "workspace.h"

#include <libinputactions/input/backend.h>

std::optional<libinputactions::CursorShape> KWinPointer::shape()
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

    // https://invent.kde.org/plasma/kwin/-/blob/d36646652272d5793eb07498db2d4e45109536fb/src/cursor.cpp#L585
    static const std::unordered_map<QString, libinputactions::CursorShape> shapes = {
        {"default", libinputactions::CursorShape::Default},
        {"up-arrow", libinputactions::CursorShape::UpArrow},
        {"crosshair", libinputactions::CursorShape::Crosshair},
        {"wait", libinputactions::CursorShape::Wait},
        {"text", libinputactions::CursorShape::Text},
        {"ns-resize", libinputactions::CursorShape::NSResize},
        {"ew-resize", libinputactions::CursorShape::EWResize},
        {"nesw-resize", libinputactions::CursorShape::NESWResize},
        {"nwse-resize", libinputactions::CursorShape::NWSEResize},
        {"all-scroll", libinputactions::CursorShape::AllScroll},
        {"row-resize", libinputactions::CursorShape::RowResize},
        {"col-resize", libinputactions::CursorShape::ColResize},
        {"pointer", libinputactions::CursorShape::Pointer},
        {"not-allowed", libinputactions::CursorShape::NotAllowed},
        {"grab", libinputactions::CursorShape::Grab},
        {"grabbing", libinputactions::CursorShape::Grabbing},
        {"help", libinputactions::CursorShape::Help},
        {"progress", libinputactions::CursorShape::Progress},
        {"move", libinputactions::CursorShape::Move},
        {"copy", libinputactions::CursorShape::Copy},
        {"alias", libinputactions::CursorShape::Alias},
        {"ne-resize", libinputactions::CursorShape::NEResize},
        {"n-resize", libinputactions::CursorShape::NResize},
        {"nw-resize", libinputactions::CursorShape::NWResize},
        {"e-resize", libinputactions::CursorShape::EResize},
        {"w-resize", libinputactions::CursorShape::WResize},
        {"se-resize", libinputactions::CursorShape::SEResize},
        {"s-resize", libinputactions::CursorShape::SResize},
        {"sw-resize", libinputactions::CursorShape::SWResize}
    };
    const QString shapeName(shapeSource->shape());
    if (shapes.contains(shapeName)) {
        m_cachedShape = shapes.at(shapeName);
        return m_cachedShape;
    }
    return {};
}

std::optional<QPointF> KWinPointer::globalPosition() const
{
    return KWin::input()->pointer()->pos();
}

std::optional<QPointF> KWinPointer::screenPosition() const
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

void KWinPointer::setPosition(const QPointF &position)
{
    auto *device = static_cast<KWinInputEmitter *>(libinputactions::InputEmitter::instance())->device();
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    Q_EMIT device->pointerMotionAbsolute(position, timestamp(), device);
    Q_EMIT device->pointerFrame(device);
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}
