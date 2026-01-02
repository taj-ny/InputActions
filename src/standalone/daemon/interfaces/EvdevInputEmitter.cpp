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

using namespace InputActions;

EvdevInputEmitter::~EvdevInputEmitter()
{
    reset();
}

void EvdevInputEmitter::initialize()
{
    auto *keyboard = libevdev_new();
    libevdev_set_name(keyboard, "InputActions Virtual Keyboard");

    libevdev_enable_event_type(keyboard, EV_KEY);
    for (const auto &key : m_keyboardRequiredKeys) {
        libevdev_enable_event_code(keyboard, EV_KEY, key, nullptr);
    }

    libevdev_uinput_create_from_device(keyboard, LIBEVDEV_UINPUT_OPEN_MANAGED, &m_keyboard);
    libevdev_free(keyboard);

    auto *mouse = libevdev_new();
    libevdev_set_name(mouse, "InputActions Virtual Mouse");

    libevdev_enable_event_type(mouse, EV_KEY);
    for (uint32_t button = BTN_LEFT; button < BTN_JOYSTICK; button++) {
        libevdev_enable_event_code(mouse, EV_KEY, button, nullptr);
    }

    libevdev_enable_event_type(mouse, EV_REL);
    libevdev_enable_event_code(mouse, EV_REL, REL_X, nullptr);
    libevdev_enable_event_code(mouse, EV_REL, REL_Y, nullptr);
    libevdev_enable_event_code(mouse, EV_REL, REL_WHEEL_HI_RES, nullptr);
    libevdev_enable_event_code(mouse, EV_REL, REL_HWHEEL_HI_RES, nullptr);

    libevdev_uinput_create_from_device(mouse, LIBEVDEV_UINPUT_OPEN_MANAGED, &m_mouse);
    libevdev_free(mouse);
}

void EvdevInputEmitter::reset()
{
    InputEmitter::reset();
    if (m_keyboard) {
        libevdev_uinput_destroy(m_keyboard);
        m_keyboard = {};
    }
    if (m_mouse) {
        libevdev_uinput_destroy(m_mouse);
        m_mouse = {};
    }
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
        libevdev_uinput_write_event(libevdevTarget, EV_KEY, key, state);
        libevdev_uinput_write_event(libevdevTarget, EV_SYN, SYN_REPORT, 0);
    } else if (m_keyboard) {
        libevdev_uinput_write_event(m_keyboard, EV_KEY, key, state);
        libevdev_uinput_write_event(m_keyboard, EV_SYN, SYN_REPORT, 0);
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
        libevdev_uinput_write_event(m_mouse, EV_REL, REL_HWHEEL_HI_RES, static_cast<int32_t>(m_mouseAxisDelta.x()));
        m_mouseAxisDelta.setX(std::fmod(m_mouseAxisDelta.x(), 1));
        syn = true;
    }
    if (std::abs(m_mouseAxisDelta.y()) > 1) {
        libevdev_uinput_write_event(m_mouse, EV_REL, REL_WHEEL_HI_RES, -static_cast<int32_t>(m_mouseAxisDelta.y()));
        m_mouseAxisDelta.setY(std::fmod(m_mouseAxisDelta.y(), 1));
        syn = true;
    }
    if (syn) {
        libevdev_uinput_write_event(m_mouse, EV_SYN, SYN_REPORT, 0);
    }
}

void EvdevInputEmitter::mouseButton(uint32_t button, bool state, const InputDevice *target)
{
    if (auto *libevdevTarget = dynamic_cast<StandaloneInputBackend *>(g_inputBackend.get())->outputDevice(target)) {
        libevdev_uinput_write_event(libevdevTarget, EV_KEY, button, state);
        libevdev_uinput_write_event(libevdevTarget, EV_SYN, SYN_REPORT, 0);
    } else if (m_mouse) {
        libevdev_uinput_write_event(m_mouse, EV_KEY, button, state);
        libevdev_uinput_write_event(m_mouse, EV_SYN, SYN_REPORT, 0);
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
        libevdev_uinput_write_event(m_mouse, EV_REL, REL_X, static_cast<int32_t>(m_mouseMotionDelta.x()));
        m_mouseMotionDelta.setX(std::fmod(m_mouseMotionDelta.x(), 1));
        syn = true;
    }
    if (std::abs(m_mouseMotionDelta.y()) > 1) {
        libevdev_uinput_write_event(m_mouse, EV_REL, REL_Y, static_cast<int32_t>(m_mouseMotionDelta.y()));
        m_mouseMotionDelta.setY(std::fmod(m_mouseMotionDelta.y(), 1));
        syn = true;
    }
    if (syn) {
        libevdev_uinput_write_event(m_mouse, EV_SYN, SYN_REPORT, 0);
    }
}

QString EvdevInputEmitter::keyboardPath() const
{
    return m_keyboard ? libevdev_uinput_get_devnode(m_keyboard) : QString();
}

QString EvdevInputEmitter::mousePath() const
{
    return m_mouse ? libevdev_uinput_get_devnode(m_mouse) : QString();
}