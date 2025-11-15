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

#include "core/inputdevice.h"
#include "input.h"
#include <libinputactions/interfaces/InputEmitter.h>

namespace InputActions
{
class InputBackend;
}

class InputDevice : public KWin::InputDevice
{
public:
    QString name() const override;
    bool isEnabled() const override;
    void setEnabled(bool enabled) override;
    bool isKeyboard() const override;
    bool isPointer() const override;
    bool isTouchpad() const override;
    bool isTouch() const override;
    bool isTabletTool() const override;
    bool isTabletPad() const override;
    bool isTabletModeSwitch() const override;
    bool isLidSwitch() const override;
};

class KWinInputEmitter : public InputActions::InputEmitter
{
public:
    KWinInputEmitter();
    ~KWinInputEmitter() override;

    void keyboardClearModifiers() override;
    void keyboardKey(uint32_t key, bool state, const InputActions::InputDevice *target = nullptr) override;
    void keyboardText(const QString &text) override;

    void mouseAxis(const QPointF &delta) override;
    void mouseButton(uint32_t button, bool state, const InputActions::InputDevice *target = nullptr) override;
    void mouseMoveRelative(const QPointF &pos) override;

    InputDevice *device() const;

private:
    KWin::InputRedirection *m_input;
    std::unique_ptr<InputDevice> m_device;
};