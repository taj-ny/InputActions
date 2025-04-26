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

#include "emitter.h"
#include "utils.h"

#include "core/output.h"
#include "keyboard_input.h"
#include "pointer_input.h"
#include "workspace.h"

#include <linux/input-event-codes.h>

static std::map<uint32_t, Qt::KeyboardModifier> s_modifiers = {
    {KEY_LEFTALT, Qt::KeyboardModifier::AltModifier},
    {KEY_LEFTCTRL, Qt::KeyboardModifier::ControlModifier},
    {KEY_LEFTMETA, Qt::KeyboardModifier::MetaModifier},
    {KEY_LEFTSHIFT, Qt::KeyboardModifier::ShiftModifier},
    {KEY_RIGHTALT, Qt::KeyboardModifier::AltModifier},
    {KEY_RIGHTCTRL, Qt::KeyboardModifier::ControlModifier},
    {KEY_RIGHTMETA, Qt::KeyboardModifier::MetaModifier},
    {KEY_RIGHTSHIFT, Qt::KeyboardModifier::ShiftModifier},
};

KWinInputEmitter::KWinInputEmitter()
    : m_device(std::make_unique<InputDevice>())
{
    auto input = KWin::input();
    input->addInputDevice(m_device.get());
    m_pointer = input->pointer();
    m_keyboard = input->keyboard();
}

KWinInputEmitter::~KWinInputEmitter()
{
    if (KWin::input()) {
        KWin::input()->removeInputDevice(m_device.get());
    }
}

void KWinInputEmitter::keyboardKey(const uint32_t &key, const bool &state)
{
    m_isEmittingInput = true;
    m_keyboard->processKey(key, state ? KeyboardKeyStatePressed : KeyboardKeyStateReleased, timestamp(), m_device.get());
    m_isEmittingInput = false;
}

void KWinInputEmitter::keyboardClearModifiers()
{
    // Prevent modifier-only global shortcuts from being triggered. Clients will still see the event and may perform
    // actions.
    const auto globalShortcutsDisabled = KWin::workspace()->globalShortcutsDisabled();
    if (!globalShortcutsDisabled) {
        KWin::workspace()->disableGlobalShortcutsForClient(true);
    }

    const auto modifiers = m_keyboard->modifiers();
    for (const auto &[key, modifier] : s_modifiers) {
        if (modifiers & modifier) {
            keyboardKey(key, false);
        }
    }

    if (!globalShortcutsDisabled) {
        KWin::workspace()->disableGlobalShortcutsForClient(false);
    }
}

void KWinInputEmitter::mouseButton(const uint32_t &button, const bool &state)
{
    m_isEmittingInput = true;
    m_pointer->processButton(button, state ? PointerButtonStatePressed : PointerButtonStateReleased, timestamp(), m_device.get());
    m_pointer->processFrame(m_device.get());
    m_isEmittingInput = false;
}

void KWinInputEmitter::mouseMoveAbsolute(const QPointF &pos)
{
    m_isEmittingInput = true;
    m_pointer->processMotionAbsolute(pos, timestamp(), m_device.get());
    m_pointer->processFrame(m_device.get());
    m_isEmittingInput = false;
}

void KWinInputEmitter::mouseMoveRelative(const QPointF &pos)
{
    m_isEmittingInput = true;
    m_pointer->processMotion(pos, pos, timestamp(), m_device.get());
    m_pointer->processFrame(m_device.get());
    m_isEmittingInput = false;
}

bool KWinInputEmitter::isEmittingInput() const
{
    return m_isEmittingInput;
}

QString InputDevice::name() const
{
    return "inputactions";
}

bool InputDevice::isEnabled() const
{
    return true;
}

void InputDevice::setEnabled(bool enabled)
{
}

bool InputDevice::isKeyboard() const
{
    return true;
}

bool InputDevice::isPointer() const
{
    return true;
}

bool InputDevice::isTouchpad() const
{
    return false;
}

bool InputDevice::isTouch() const
{
    return false;
}

bool InputDevice::isTabletTool() const
{
    return false;
}

bool InputDevice::isTabletPad() const
{
    return false;
}

bool InputDevice::isTabletModeSwitch() const
{
    return false;
}

bool InputDevice::isLidSwitch() const
{
    return false;
}

#ifndef KWIN_6_3_OR_GREATER
QString InputDevice::sysName() const
{
    return name();
}

KWin::LEDs InputDevice::leds() const
{
    return KWin::LEDs::fromInt(0);
}

void InputDevice::setLeds(KWin::LEDs leds)
{
}
#endif