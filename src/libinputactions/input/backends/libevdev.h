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

#pragma once

// i hate cmake, why does this have a version, it'll probably break at some point
#include <libevdev-1.0/libevdev/libevdev.h>

#include <thread>

#include <QSize>

namespace libinputactions
{

struct TouchpadDevice
{
    libevdev *device;
    int fd;
    QSize size;
};

class LibevdevComplementaryInputBackend
{
public:
    LibevdevComplementaryInputBackend();
    ~LibevdevComplementaryInputBackend();

private:
    void inputLoop();

    std::vector<TouchpadDevice> m_devices;
    std::thread m_inputLoopThread;
};
}