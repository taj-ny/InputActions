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
#include <QThread>
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/devices/VirtualKeyboard.h>
#include <libinputactions/input/devices/VirtualMouse.h>
#include <libinputactions/interfaces/PointerPositionSetter.h>
#include <libinputactions/utils/ThreadUtils.h>

namespace InputActions
{

InputAction::InputAction(std::vector<Item> sequence)
    : m_sequence(std::move(sequence))
{
    for (const auto &item : m_sequence) {
        if (item.keyboardPress) {
            g_inputBackend->addVirtualKeyboardKey(item.keyboardPress);
        }
        if (item.keyboardRelease) {
            g_inputBackend->addVirtualKeyboardKey(item.keyboardRelease);
        }
    }
}

void InputAction::executeImpl(uint32_t executions)
{
    for (const auto &item : m_sequence) {
        const auto keyboardText = item.keyboardText.get();
        ThreadUtils::runOnThread(ThreadUtils::mainThread(), [this, executions, item, keyboardText]() {
            if (item.keyboardPress) {
                g_inputBackend->virtualKeyboard()->keyboardKey(item.keyboardPress, true);
            } else if (item.keyboardRelease) {
                g_inputBackend->virtualKeyboard()->keyboardKey(item.keyboardRelease, false);
            } else if (keyboardText) {
                g_inputBackend->virtualKeyboard()->keyboardText(keyboardText.value());
            } else if (item.mousePress) {
                g_inputBackend->virtualMouse()->mouseButton(item.mousePress, true);
            } else if (item.mouseRelease) {
                g_inputBackend->virtualMouse()->mouseButton(item.mouseRelease, false);
            } else if (!item.mouseAxis.isNull()) {
                g_inputBackend->virtualMouse()->mouseWheel(item.mouseAxis * executions);
            } else if (!item.mouseMoveAbsolute.isNull()) {
                g_pointerPositionSetter->setGlobalPointerPosition(item.mouseMoveAbsolute);
            } else if (!item.mouseMoveRelative.isNull()) {
                g_inputBackend->virtualMouse()->mouseMotion(item.mouseMoveRelative);
            } else if (item.mouseMoveRelativeByDelta) {
                g_inputBackend->virtualMouse()->mouseMotion(m_deltaMultiplied);
            }
        });

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

bool InputAction::mergeable() const
{
    return !m_delay.count() && std::ranges::all_of(m_sequence, [](const auto &item) {
        return !item.mouseAxis.isNull();
    });
}

}