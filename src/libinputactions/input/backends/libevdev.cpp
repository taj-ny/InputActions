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

#include "libevdev.h"

#include <libinputactions/variables/manager.h>

#include <fcntl.h>

#include <QDir>
#include <QLoggingCategory>

namespace libinputactions
{

Q_LOGGING_CATEGORY(LIBINPUTACTIONS_BACKEND_LIBEVDEV, "libinputactions.input.backend.libevdev", QtWarningMsg)

LibevdevComplementaryInputBackend::LibevdevComplementaryInputBackend()
{
    for (const auto &name : QDir("/dev/input").entryList(QDir::Filter::System).filter("event")) {
        const auto path = QString("/dev/input/%1").arg(name).toStdString();
        const auto fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd == -1) {
            qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote() << QString("Failed to open %1 (error %2)").arg(path, fd);
            continue;
        }

        libevdev *device;
        libevdev_new_from_fd(fd, &device);
        if (!libevdev_has_event_type(device, EV_ABS)) {
            // Probably not a touchpad
            libevdev_free(device);
            close(fd);
            continue;
        }

        QSize size(libevdev_get_abs_maximum(device, ABS_X), libevdev_get_abs_maximum(device, ABS_Y));
        if (size.width() == 0 || size.height() == 0) {
            libevdev_free(device);
            close(fd);
            qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV, "Device has a size of 0");
            continue;
        }

        qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV) << "Found valid touchpad, size: " << size;
        m_devices.push_back({
            .device = device,
            .fd = fd,
            .size = size
        });
    }

    if (m_devices.empty()) {
        return;
    }

    VariableManager::instance()->registerLocalVariable<qreal>("finger_1_position_percentage_x");
    VariableManager::instance()->registerLocalVariable<qreal>("finger_1_position_percentage_y");

    m_inputLoopThread = std::thread([this] {
        inputLoop();
    });
    m_inputLoopThread.detach();
}

LibevdevComplementaryInputBackend::~LibevdevComplementaryInputBackend()
{
    auto devicesCopy = m_devices;
    m_devices.clear();
    for (const auto &device : devicesCopy) {
        close(device.fd);
        libevdev_free(device.device);
    }
}

void LibevdevComplementaryInputBackend::inputLoop()
{
    while (!m_devices.empty()) {
        input_event event;
        bool pending{};
        for (const auto &device : m_devices) {
            if (libevdev_next_event(device.device, LIBEVDEV_READ_FLAG_NORMAL, &event) != 0
                || event.type != EV_ABS) {
                continue;
            }

            // not thread safe
            switch (event.code) {
                case ABS_X:
                    VariableManager::instance()->getVariable<qreal>("finger_1_position_percentage_x")->set(event.value / static_cast<qreal>(device.size.width()));
                    break;
                case ABS_Y:
                    VariableManager::instance()->getVariable<qreal>("finger_1_position_percentage_y")->set(event.value / static_cast<qreal>(device.size.height()));
                    break;
            }

            if (!pending) {
                pending = libevdev_has_event_pending(device.device);
            }
        }

        if (!pending) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

}