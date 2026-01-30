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

#pragma once

#include <libevdev-cpp/LibevdevUinputDevice.h>
#include <libinputactions/input/devices/VirtualKeyboard.h>

namespace InputActions
{

class EvdevVirtualKeyboard : public VirtualKeyboard
{
public:
    EvdevVirtualKeyboard(const std::set<uint32_t> &keys);
    ~EvdevVirtualKeyboard() override;

    QString path() const;

    void keyboardKey(uint32_t key, bool state) override;

private:
    std::optional<LibevdevUinputDevice> m_device;
};

}