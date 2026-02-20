/*
    Input Actions - Input handler that executes user-defined actions
    Copyright (C) 2024-2026 Marcin Woźniak

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

#include "KWinInputDevice.h"
#include "KWinInputBackend.h"
#include "input_event.h"
#include "utils.h"

namespace InputActions
{

KWinInputDevice::KWinInputDevice(KWinInputBackend *backend, KWin::InputDevice *device, InputDeviceType type)
    : InputDevice(type, device->name(), device->property("sysName").toString())
    , m_kwinDevice(device)
    , m_backend(backend)
{
    if (device->property("lmrTapButtonMap").value<bool>()) {
        properties().setTouchpadLmrTapButtonMap(true);
    }
    if (type == InputDeviceType::Touchpad || type == InputDeviceType::Touchscreen) {
        properties().setSize(device->property("size").value<QSizeF>());
    }
}

std::unique_ptr<KWinInputDevice> KWinInputDevice::tryCreate(KWinInputBackend *backend, KWin::InputDevice *device)
{
    InputDeviceType type;
    if (device->isPointer() && !device->isTouch() && !device->isTouchpad()) {
        type = InputDeviceType::Mouse;
    } else if (device->isKeyboard()) {
        type = InputDeviceType::Keyboard;
    } else if (device->isTouchpad()) {
        type = InputDeviceType::Touchpad;
    } else if (device->isTouch()) {
        type = InputDeviceType::Touchscreen;
    } else {
        return {};
    }

    return std::unique_ptr<KWinInputDevice>(new KWinInputDevice(backend, device, type));
}

void KWinInputDevice::keyboardKey(KeyboardKey key, bool state)
{
    g_inputBackend->setIgnoreEvents(true);
    Q_EMIT m_kwinDevice->keyChanged(key.scanCode(), state ? KWin::KeyboardKeyState::Pressed : KWin::KeyboardKeyState::Released, timestamp(), m_kwinDevice);
    InputDevice::keyboardKey(key, state);
    g_inputBackend->setIgnoreEvents(false);
}

void KWinInputDevice::mouseButton(MouseButton button, bool state)
{
    g_inputBackend->setIgnoreEvents(true);
    Q_EMIT m_kwinDevice->pointerButtonChanged(button.scanCode(),
                                              state ? KWin::PointerButtonState::Pressed : KWin::PointerButtonState::Released,
                                              timestamp(),
                                              m_kwinDevice);
    Q_EMIT m_kwinDevice->pointerFrame(m_kwinDevice);
    g_inputBackend->setIgnoreEvents(false);
}

// Events generated during resetting and restoring must not go through TouchInputRedirection, as it would interfere with the physical state.
#ifdef KWIN_6_5_OR_GREATER
void KWinInputDevice::resetVirtualDeviceState()
{
    if (type() != InputDeviceType::Touchscreen) {
        return;
    }

    m_backend->setIgnoreEvents(true);

    for (const auto *point : physicalState().validTouchPoints()) {
        KWin::TouchUpEvent event{
            .id = point->id,
            .time = timestamp(),
        };
        KWin::input()->processSpies(&KWin::InputEventSpy::touchUp, &event);
        KWin::input()->processFilters(&KWin::InputEventFilter::touchUp, &event);
    }
    KWin::input()->processFilters(&KWin::InputEventFilter::touchFrame);

    m_backend->setIgnoreEvents(false);
}

void KWinInputDevice::restoreVirtualDeviceState()
{
    if (type() != InputDeviceType::Touchscreen) {
        return;
    }

    m_backend->setIgnoreEvents(true);

    for (const auto *point : physicalState().validTouchPoints()) {
        KWin::TouchDownEvent event{
            .id = point->id,
            .pos = point->rawInitialPosition,
            .time = timestamp(),
        };
        KWin::input()->processSpies(&KWin::InputEventSpy::touchDown, &event);
        KWin::input()->processFilters(&KWin::InputEventFilter::touchDown, &event);
    }
    KWin::input()->processFilters(&KWin::InputEventFilter::touchFrame);

    for (const auto *point : physicalState().validTouchPoints()) {
        KWin::TouchMotionEvent event{
            .id = point->id,
            .pos = point->rawPosition,
            .time = timestamp(),
        };
        KWin::input()->processSpies(&KWin::InputEventSpy::touchMotion, &event);
        KWin::input()->processFilters(&KWin::InputEventFilter::touchMotion, &event);
    }
    KWin::input()->processFilters(&KWin::InputEventFilter::touchFrame);

    m_backend->setIgnoreEvents(false);
}

void KWinInputDevice::touchscreenTapDown(const std::vector<QPointF> &points)
{
    m_backend->setIgnoreEvents(true);

    for (size_t i = 0; i < points.size(); ++i) {
        Q_EMIT m_kwinDevice->touchDown(i, points[i], timestamp(), m_kwinDevice);
    }
    Q_EMIT m_kwinDevice->touchFrame(m_kwinDevice);

    m_backend->setIgnoreEvents(false);
}

void KWinInputDevice::touchscreenTapUp(const std::vector<QPointF> &points)
{
    m_backend->setIgnoreEvents(true);

    for (size_t i = 0; i < points.size(); ++i) {
        Q_EMIT m_kwinDevice->touchUp(i, timestamp(), m_kwinDevice);
    }
    Q_EMIT m_kwinDevice->touchFrame(m_kwinDevice);

    m_backend->setIgnoreEvents(false);
}
#endif

}