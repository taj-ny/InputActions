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

#include "core/output.h"
#include "pointer_input.h"
#include "workspace.h"

#include <libinputactions/input/backend.h>

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
    auto pointer = KWin::input()->pointer();
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    pointer->processMotionAbsolute(position, timestamp(), device);
    pointer->processFrame(device);
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}