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

#pragma once

#include "EvdevInputEmitter.h"
#include "protocols/VirtualKeyboardUnstableV1.h"
#include "protocols/WlrVirtualPointerUnstableV1.h"

class StandaloneInputEmitter : public EvdevInputEmitter
{
public:
    StandaloneInputEmitter();

    void keyboardKey(uint32_t key, bool state, const InputActions::InputDevice *target = nullptr) override;

    void mouseButton(uint32_t button, bool state, const InputActions::InputDevice *target = nullptr) override;
    void mouseMoveRelative(const QPointF &delta) override;

private:
    std::unique_ptr<VirtualKeyboardUnstableV1Keyboard> m_virtualKeyboardUnstableV1Keyboard;
    std::unique_ptr<WlrVirtualPointerUnstableV1Pointer> m_wlrVirtualPointerUnstableV1Pointer;

    Qt::KeyboardModifiers m_modifiers{};
};