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

#include "KWinInputEmitter.h"
#include "globals.h"
#include "utils.h"

#include <libinputactions/input/backends/InputBackend.h>

#include <kwin/input_event_spy.h>

KWinInputEmitter::KWinInputEmitter()
    : m_input(KWin::input())
    , m_device(std::make_unique<InputDevice>())
{
    m_input->addInputDevice(m_device.get());
}

KWinInputEmitter::~KWinInputEmitter()
{
    if (KWin::input()) {
        m_input->removeInputDevice(m_device.get());
    }
}

void KWinInputEmitter::keyboardKey(const uint32_t &key, const bool &state)
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    Q_EMIT m_device->keyChanged(key, state ? KeyboardKeyStatePressed : KeyboardKeyStateReleased, timestamp(), m_device.get());
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

void KWinInputEmitter::mouseButton(const uint32_t &button, const bool &state)
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    Q_EMIT m_device->pointerButtonChanged(button, state ? PointerButtonStatePressed : PointerButtonStateReleased, timestamp(), m_device.get());
    Q_EMIT m_device->pointerFrame(m_device.get());
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

void KWinInputEmitter::mouseMoveRelative(const QPointF &pos)
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    Q_EMIT m_device->pointerMotion(pos, pos, timestamp(), m_device.get());
    Q_EMIT m_device->pointerFrame(m_device.get());
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

void KWinInputEmitter::touchpadPinchBegin(const uint8_t &fingers)
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    const auto time = timestamp();
    m_input->processSpies([&fingers, &time](auto &&spy) {
        spy->pinchGestureBegin(fingers, time);
    });
    m_input->processFilters([&fingers, &time](auto &&filter) {
        return filter->pinchGestureBegin(fingers, time);
    });
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

void KWinInputEmitter::touchpadSwipeBegin(const uint8_t &fingers)
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    const auto time = timestamp();
    m_input->processSpies([&fingers, &time](auto &&spy) {
        spy->swipeGestureBegin(fingers, time);
    });
    m_input->processFilters([&fingers, &time](auto &&filter) {
        return filter->swipeGestureBegin(fingers, time);
    });
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

InputDevice *KWinInputEmitter::device() const
{
    return m_device.get();
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