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

#include "EvdevVirtualKeyboard.h"
#include <libevdev-cpp/Device.h>
#include <libevdev-cpp/exceptions.h>

namespace InputActions
{

EvdevVirtualKeyboard::EvdevVirtualKeyboard(const std::set<uint32_t> &keys)
{
    libevdev::Device device;
    device.enableEventType(EV_KEY);
    for (const auto key : keys) {
        device.enableEventCode(EV_KEY, key, nullptr);
    }

    try {
        m_device = libevdev::UInputDevice::createManaged(&device, "InputActions Virtual Keyboard");
    } catch (const libevdev::Exception &) {
    }
}

EvdevVirtualKeyboard::~EvdevVirtualKeyboard()
{
    reset();
}

QString EvdevVirtualKeyboard::path() const
{
    return m_device ? m_device->devNode() : QString();
}

void EvdevVirtualKeyboard::keyboardKey(uint32_t key, bool state)
{
    if (!m_device) {
        return;
    }

    m_device->writeEvent(EV_KEY, key, state);
    m_device->writeSynReportEvent();
    VirtualKeyboard::keyboardKey(key, state);
}

}