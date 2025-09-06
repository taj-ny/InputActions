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

#include "InputAction.h"
#include "InputActions.h"
#include <QThread>
#include <libinputactions/interfaces/InputEmitter.h>
#include <libinputactions/interfaces/PointerPositionSetter.h>

namespace libinputactions
{

InputAction::InputAction(std::vector<Item> sequence)
    : m_sequence(std::move(sequence))
{
}

void InputAction::executeImpl()
{
    for (const auto &item : m_sequence) {
        const auto keyboardText = item.keyboardText.get();
        InputActions::runOnMainThread(
            [this, item, keyboardText]() {
                if (item.keyboardPress) {
                    g_inputEmitter->keyboardKey(item.keyboardPress, true);
                } else if (item.keyboardRelease) {
                    g_inputEmitter->keyboardKey(item.keyboardRelease, false);
                } else if (keyboardText) {
                    g_inputEmitter->keyboardText(keyboardText.value());
                } else if (item.mousePress) {
                    g_inputEmitter->mouseButton(item.mousePress, true);
                } else if (item.mouseRelease) {
                    g_inputEmitter->mouseButton(item.mouseRelease, false);
                } else if (!item.mouseMoveAbsolute.isNull()) {
                    g_pointerPositionSetter->setGlobalPointerPosition(item.mouseMoveAbsolute);
                } else if (!item.mouseMoveRelative.isNull()) {
                    g_inputEmitter->mouseMoveRelative(item.mouseMoveRelative);
                } else if (item.mouseMoveRelativeByDelta) {
                    g_inputEmitter->mouseMoveRelative(m_deltaMultiplied);
                }
            },
            false);

        if (m_delay.count()) {
            QThread::msleep(m_delay.count());
        }
    }
}

bool InputAction::async() const
{
    return m_delay.count() || std::ranges::any_of(m_sequence, [](const auto &item) {
               return item.keyboardText.expensive();
           });
}

}