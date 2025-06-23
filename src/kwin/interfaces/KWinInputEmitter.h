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

#include <libinputactions/interfaces/InputEmitter.h>

#include "core/inputdevice.h"
#include "kwin/input.h"
#include "input.h"

namespace libinputactions
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

#ifndef KWIN_6_3_OR_GREATER
    QString sysName() const override;
    KWin::LEDs leds() const override;
    void setLeds(KWin::LEDs leds) override;
#endif
};

class KWinInputEmitter : public libinputactions::InputEmitter
{
public:
    KWinInputEmitter();
    ~KWinInputEmitter() override;

    void keyboardClearModifiers() override;
    void keyboardKey(const uint32_t &key, const bool &state) override;

    void mouseButton(const uint32_t &button, const bool &state) override;
    void mouseMoveRelative(const QPointF &pos) override;

    void touchpadPinchBegin(const uint8_t &fingers) override;
    void touchpadSwipeBegin(const uint8_t &fingers) override;

    InputDevice *device() const;

private:
    KWin::InputRedirection *m_input;
    std::unique_ptr<InputDevice> m_device;
};