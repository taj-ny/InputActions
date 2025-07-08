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

#include <libinputactions/actions/InputTriggerAction.h>
#include <libinputactions/interfaces/InputEmitter.h>
#include <libinputactions/interfaces/PointerPositionSetter.h>

namespace libinputactions
{

void InputTriggerAction::execute()
{
    const auto input = InputEmitter::instance();
    for (const auto &action : m_sequence) {
        for (const auto &key : action.keyboardPress) {
            input->keyboardKey(key, true);
        }
        for (const auto &key : action.keyboardRelease) {
            input->keyboardKey(key, false);
        }
        const auto keyboardText = action.keyboardText.get();
        if (!keyboardText.isEmpty()) {
            input->keyboardText(keyboardText);
        }

        for (const auto &button : action.mousePress) {
            input->mouseButton(button, true);
        }
        for (const auto &button : action.mouseRelease) {
            input->mouseButton(button, false);
        }

        if (!action.mouseMoveAbsolute.isNull()) {
            PointerPositionSetter::instance()->setGlobalPointerPosition(action.mouseMoveAbsolute);
        }
        if (!action.mouseMoveRelative.isNull()) {
            input->mouseMoveRelative(action.mouseMoveRelative);
        }
        if (action.mouseMoveRelativeByDelta) {
            input->mouseMoveRelative(m_currentDeltaPointMultiplied);
        }
    }
}

void InputTriggerAction::setSequence(const std::vector<InputAction> &sequence)
{
    m_sequence = sequence;
}

}