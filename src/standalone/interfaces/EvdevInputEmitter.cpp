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

#include <libinputactions/input/Keyboard.h>

#include <fcntl.h>
#include <linux/uinput.h>

using namespace libinputactions;

EvdevInputEmitter::~EvdevInputEmitter()
{
    reset();
}

void EvdevInputEmitter::initialize()
{
    m_keyboardFd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    fcntl(m_keyboardFd, F_SETFD, FD_CLOEXEC);
    ioctl(m_keyboardFd, UI_SET_EVBIT, EV_KEY);
    for (const auto &key : m_keyboardRequiredKeys) {
        ioctl(m_keyboardFd, UI_SET_KEYBIT, key);
    }
    uinput_setup setup{};
    setup.id.bustype = BUS_USB;
    strcpy(setup.name, "inputactions_keyboard");
    ioctl(m_keyboardFd, UI_DEV_SETUP, &setup);
    ioctl(m_keyboardFd, UI_DEV_CREATE);

    m_mouseFd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    fcntl(m_mouseFd, F_SETFD, FD_CLOEXEC);
    ioctl(m_mouseFd, UI_SET_EVBIT, EV_KEY);
    for (uint32_t button = BTN_LEFT; button < BTN_JOYSTICK; button++) {
        ioctl(m_mouseFd, UI_SET_KEYBIT, button);
    }
    ioctl(m_mouseFd, UI_SET_EVBIT, EV_REL);
    ioctl(m_mouseFd, UI_SET_RELBIT, REL_X);
    ioctl(m_mouseFd, UI_SET_RELBIT, REL_Y);
    setup = {};
    setup.id.bustype = BUS_USB;
    strcpy(setup.name, "inputactions_mouse");
    ioctl(m_mouseFd, UI_DEV_SETUP, &setup);
    ioctl(m_mouseFd, UI_DEV_CREATE);
}

void EvdevInputEmitter::reset()
{
    libinputactions::InputEmitter::reset();
    uinputDestroyDevice(m_keyboardFd);
    uinputDestroyDevice(m_mouseFd);
}

void EvdevInputEmitter::keyboardClearModifiers()
{
    const auto modifiers = Keyboard::instance()->modifiers();
    for (const auto &[key, modifier] : MODIFIERS) {
        if (modifiers & modifier) {
            keyboardKey(key, false);
        }
    }
}

void EvdevInputEmitter::keyboardKey(const uint32_t &key, const bool &state)
{
    uinputEmit(m_keyboardFd, EV_KEY, key, state);
    uinputEmit(m_keyboardFd, EV_SYN, SYN_REPORT);
}

void EvdevInputEmitter::mouseButton(const uint32_t &button, const bool &state)
{
    uinputEmit(m_mouseFd, EV_KEY, button, state);
    uinputEmit(m_mouseFd, EV_SYN, SYN_REPORT);
}

void EvdevInputEmitter::mouseMoveRelative(const QPointF &pos)
{
    m_mouseDelta += pos;
    auto syn = false;
    if (std::abs(m_mouseDelta.x()) > 1) {
        uinputEmit(m_mouseFd, EV_REL, REL_X, static_cast<int32_t>(m_mouseDelta.x()));
        m_mouseDelta.setX(std::fmod(m_mouseDelta.x(), 1));
        syn = true;
    }
    if (std::abs(m_mouseDelta.y()) > 1) {
        uinputEmit(m_mouseFd, EV_REL, REL_Y, static_cast<int32_t>(m_mouseDelta.y()));
        m_mouseDelta.setY(std::fmod(m_mouseDelta.y(), 1));
        syn = true;
    }
    if (syn) {
        uinputEmit(m_mouseFd, EV_SYN, SYN_REPORT);
    }
}

void EvdevInputEmitter::uinputDestroyDevice(int &fd)
{
    if (fd == -1) {
        return;
    }
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
}

void EvdevInputEmitter::uinputEmit(int fd, uint16_t type, uint16_t code, int32_t value)
{
    input_event event{
        .time = {
            .tv_sec = 0,
            .tv_usec = 0
        },
        .type = type,
        .code = code,
        .value = value
    };
    write(fd, &event, sizeof(event));
}