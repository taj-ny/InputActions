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

void InputAction::execute() const
{
    for (const auto &item : m_sequence) {
        const auto keyboardText = item.keyboardText.get();
        g_inputActions->runOnMainThread(
            [this, item, keyboardText]() {
                for (const auto &key : item.keyboardPress) {
                    g_inputEmitter->keyboardKey(key, true);
                }
                for (const auto &key : item.keyboardRelease) {
                    g_inputEmitter->keyboardKey(key, false);
                }
                if (!keyboardText.isEmpty()) {
                    g_inputEmitter->keyboardText(keyboardText);
                }

                for (const auto &button : item.mousePress) {
                    g_inputEmitter->mouseButton(button, true);
                }
                for (const auto &button : item.mouseRelease) {
                    g_inputEmitter->mouseButton(button, false);
                }

                if (!item.mouseMoveAbsolute.isNull()) {
                    g_pointerPositionSetter->setGlobalPointerPosition(item.mouseMoveAbsolute);
                }
                if (!item.mouseMoveRelative.isNull()) {
                    g_inputEmitter->mouseMoveRelative(item.mouseMoveRelative);
                }
                if (item.mouseMoveRelativeByDelta) {
                    g_inputEmitter->mouseMoveRelative(m_deltaMultiplied);
                }
            },
            false);
    }
}

bool InputAction::async() const
{
    return std::ranges::any_of(m_sequence, [](const auto &item) {
        return item.keyboardText.source() == ValueSource::Command;
    });
}

}