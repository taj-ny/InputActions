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

#include "EvdevInputEmitter.h"
#include "input/StandaloneInputBackend.h"
#include <libevdev-cpp/LibevdevDevice.h>
#include <libevdev-cpp/LibevdevUinputDevice.h>

namespace InputActions
{

EvdevInputEmitter::EvdevInputEmitter() = default;
EvdevInputEmitter::~EvdevInputEmitter() = default;

void EvdevInputEmitter::initialize()
{
    LibevdevDevice keyboard;
    keyboard.enableEventType(EV_KEY);
    for (const auto &key : m_keyboardRequiredKeys) {
        keyboard.enableEventCode(EV_KEY, key, nullptr);
    }

    m_keyboard = LibevdevUinputDevice::createManaged(&keyboard, "InputActions Virtual Keyboard");

    LibevdevDevice mouse;
    mouse.enableEventType(EV_KEY);
    for (uint32_t button = BTN_LEFT; button < BTN_JOYSTICK; button++) {
        mouse.enableEventCode(EV_KEY, button, nullptr);
    }

    mouse.enableEventType(EV_REL);
    mouse.enableEventCode(EV_REL, REL_X, nullptr);
    mouse.enableEventCode(EV_REL, REL_Y, nullptr);
    mouse.enableEventCode(EV_REL, REL_WHEEL_HI_RES, nullptr);
    mouse.enableEventCode(EV_REL, REL_HWHEEL_HI_RES, nullptr);

    m_mouse = LibevdevUinputDevice::createManaged(&mouse, "InputActions Virtual Mouse");
}

void EvdevInputEmitter::reset()
{
    InputEmitter::reset();
    m_keyboard = {};
    m_mouse = {};
}

void EvdevInputEmitter::keyboardClearModifiers()
{
    for (auto *device : g_inputBackend->devices()) {
        if (device->type() != InputDeviceType::Keyboard) {
            continue;
        }

        for (const auto &[key, modifier] : KEYBOARD_MODIFIERS) {
            if (device->modifiers() & modifier) {
                keyboardKey(key, false, device);
            }
        }
    }
}

void EvdevInputEmitter::keyboardKey(uint32_t key, bool state, const InputDevice *target)
{
    if (auto *libevdevTarget = dynamic_cast<StandaloneInputBackend *>(g_inputBackend.get())->outputDevice(target)) {
        libevdevTarget->writeEvent(EV_KEY, key, state);
        libevdevTarget->writeSynReportEvent();
    } else if (m_keyboard) {
        m_keyboard->writeEvent(EV_KEY, key, state);
        m_keyboard->writeSynReportEvent();
    }
}

void EvdevInputEmitter::mouseAxis(const QPointF &delta)
{
    if (!m_mouse) {
        return;
    }

    m_mouseAxisDelta += delta;
    auto syn = false;
    if (std::abs(m_mouseAxisDelta.x()) > 1) {
        m_mouse->writeEvent(EV_REL, REL_HWHEEL_HI_RES, static_cast<int32_t>(m_mouseAxisDelta.x()));
        m_mouseAxisDelta.setX(std::fmod(m_mouseAxisDelta.x(), 1));
        syn = true;
    }
    if (std::abs(m_mouseAxisDelta.y()) > 1) {
        m_mouse->writeEvent(EV_REL, REL_WHEEL_HI_RES, -static_cast<int32_t>(m_mouseAxisDelta.y()));
        m_mouseAxisDelta.setY(std::fmod(m_mouseAxisDelta.y(), 1));
        syn = true;
    }
    if (syn) {
        m_mouse->writeSynReportEvent();
    }
}

void EvdevInputEmitter::mouseButton(uint32_t button, bool state, const InputDevice *target)
{
    if (auto *libevdevTarget = dynamic_cast<StandaloneInputBackend *>(g_inputBackend.get())->outputDevice(target)) {
        libevdevTarget->writeEvent(EV_KEY, button, state);
        libevdevTarget->writeSynReportEvent();
    } else if (m_mouse) {
        m_mouse->writeEvent(EV_KEY, button, state);
        m_mouse->writeSynReportEvent();
    }
}

void EvdevInputEmitter::mouseMoveRelative(const QPointF &pos)
{
    if (!m_mouse) {
        return;
    }

    m_mouseMotionDelta += pos;
    auto syn = false;
    if (std::abs(m_mouseMotionDelta.x()) > 1) {
        m_mouse->writeEvent(EV_REL, REL_X, static_cast<int32_t>(m_mouseMotionDelta.x()));
        m_mouseMotionDelta.setX(std::fmod(m_mouseMotionDelta.x(), 1));
        syn = true;
    }
    if (std::abs(m_mouseMotionDelta.y()) > 1) {
        m_mouse->writeEvent(EV_REL, REL_Y, static_cast<int32_t>(m_mouseMotionDelta.y()));
        m_mouseMotionDelta.setY(std::fmod(m_mouseMotionDelta.y(), 1));
        syn = true;
    }
    if (syn) {
        m_mouse->writeSynReportEvent();
    }
}

QString EvdevInputEmitter::keyboardPath() const
{
    return m_keyboard ? m_keyboard->devNode() : QString();
}

QString EvdevInputEmitter::mousePath() const
{
    return m_mouse ? m_mouse->devNode() : QString();
}

}