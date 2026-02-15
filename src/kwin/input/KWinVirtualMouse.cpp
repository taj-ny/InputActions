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

#include "KWinVirtualMouse.h"
#include "utils.h"
#include <libinputactions/input/backends/InputBackend.h>

namespace InputActions
{

KWinVirtualMouse::KWinVirtualMouse()
{
    KWin::input()->addInputDevice(&m_device);
}

KWinVirtualMouse::~KWinVirtualMouse()
{
    reset();
    if (auto *input = KWin::input()) {
        input->removeInputDevice(&m_device);
    }
}

void KWinVirtualMouse::mouseButton(MouseButton button, bool state)
{
    g_inputBackend->setIgnoreEvents(true);
    Q_EMIT m_device.pointerButtonChanged(button.scanCode(),
                                         state ? KWin::PointerButtonState::Pressed : KWin::PointerButtonState::Released,
                                         timestamp(),
                                         &m_device);
    Q_EMIT m_device.pointerFrame(&m_device);
    VirtualMouse::mouseButton(button, state);
    g_inputBackend->setIgnoreEvents(false);
}

void KWinVirtualMouse::mouseMotion(const QPointF &pos)
{
    g_inputBackend->setIgnoreEvents(true);
    Q_EMIT m_device.pointerMotion(pos, pos, timestamp(), &m_device);
    Q_EMIT m_device.pointerFrame(&m_device);
    g_inputBackend->setIgnoreEvents(false);
}

void KWinVirtualMouse::mouseWheel(const QPointF &delta)
{
    g_inputBackend->setIgnoreEvents(true);
    if (delta.x()) {
        Q_EMIT m_device.pointerAxisChanged(KWin::PointerAxis::Horizontal, delta.x(), delta.x(), KWin::PointerAxisSource::Wheel, false, timestamp(), &m_device);
    }
    if (delta.y()) {
        Q_EMIT m_device.pointerAxisChanged(KWin::PointerAxis::Vertical, delta.y(), delta.y(), KWin::PointerAxisSource::Wheel, false, timestamp(), &m_device);
    }
    Q_EMIT m_device.pointerFrame(&m_device);
    g_inputBackend->setIgnoreEvents(false);
}

}