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
#include <libinputactions/input/Keyboard.h>

#include <kwin/input_event_spy.h>
#include <kwin/wayland_server.h>
#include <kwin/wayland/seat.h>
#include <kwin/wayland/textinput_v1.h>
#include <kwin/wayland/textinput_v2.h>
#include <kwin/wayland/textinput_v3.h>
#include <kwin/workspace.h>

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

void KWinInputEmitter::keyboardClearModifiers()
{
    // Prevent modifier-only global shortcuts from being triggered. Clients will still see the event and may perform actions.
    const auto globalShortcutsDisabled = KWin::workspace()->globalShortcutsDisabled();
    if (!globalShortcutsDisabled) {
        KWin::workspace()->disableGlobalShortcutsForClient(true);
    }

    auto emitter = InputEmitter::instance();
    const auto modifiers = libinputactions::Keyboard::instance()->modifiers(); // This is not the real state, but it's fine in this case.
    for (const auto &[key, modifier] : libinputactions::MODIFIERS) {
        if (modifiers & modifier) {
            emitter->keyboardKey(key, false);
        }
    }

    if (!globalShortcutsDisabled) {
        KWin::workspace()->disableGlobalShortcutsForClient(false);
    }
}

void KWinInputEmitter::keyboardKey(const uint32_t &key, const bool &state)
{
    libinputactions::InputBackend::instance()->setIgnoreEvents(true);
    Q_EMIT m_device->keyChanged(key, state ? KeyboardKeyStatePressed : KeyboardKeyStateReleased, timestamp(), m_device.get());
    libinputactions::InputBackend::instance()->setIgnoreEvents(false);
}

void KWinInputEmitter::keyboardText(const QString &text)
{
    auto *seat = KWin::waylandServer()->seat();
    auto *v1 = seat->textInputV1();
    auto *v2 = seat->textInputV2();
    auto *v3 = seat->textInputV3();

    if (v3->isEnabled()) {
        v3->sendPreEditString({}, 0, 0);
        v3->commitString(text);
        v3->done();
    } else if (v2->isEnabled()) {
        v2->commitString(text);
        v2->setPreEditCursor(0);
        v2->preEdit({}, {});
    } else if (v1->isEnabled()) {
        v1->commitString(text);
        v1->setPreEditCursor(0);
        v1->preEdit({}, {});
    }
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