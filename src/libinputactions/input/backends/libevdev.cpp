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
    m_inputTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_inputTimer.setInterval(100);
    connect(&m_inputTimer, &QTimer::timeout, this, [this] {
        processEvents();
    });

    m_devInputWatcher.addPath("/dev/input");
    connect(&m_devInputWatcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV, "/dev/input changed");
        removeDevices();
        addDevices();
    });

    addDevices();
}

LibevdevComplementaryInputBackend::~LibevdevComplementaryInputBackend()
{
    removeDevices();
}

void LibevdevComplementaryInputBackend::addDevices()
{
    for (const auto &name: QDir("/dev/input").entryList(QDir::Filter::System).filter("event")) {
        const auto path = QString("/dev/input/%1").arg(name).toStdString();
        const auto fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
        if (fd == -1) {
            qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote()
                << QString("Failed to open %1 (error %2)").arg(QString::fromStdString(path), QString::number(errno));
            continue;
        }

        libevdev *device;
        libevdev_new_from_fd(fd, &device);
        qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace() << "Opened device (name: " << libevdev_get_name(device) << ")";

        if (!libevdev_has_event_type(device, EV_ABS)) {
            qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV, "Device is not absolute");
            libevdev_free(device);
            close(fd);
            continue;
        }

        const QSize size(libevdev_get_abs_maximum(device, ABS_X), libevdev_get_abs_maximum(device, ABS_Y));
        if (size.width() == 0 || size.height() == 0) {
            qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV, "Device has a size of 0");
            libevdev_free(device);
            close(fd);
            continue;
        }

        bool multiTouch{};
        uint8_t slotCount = 1;
        if (libevdev_has_event_code(device, EV_ABS, ABS_MT_SLOT)) {
            multiTouch = true;
            slotCount = libevdev_get_abs_maximum(device, ABS_MT_SLOT);
        }
        qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace()
            << "Found valid touchpad (size: " << size << ", multiTouch: " << multiTouch << ", slots: " << slotCount << ")";
        TouchpadDevice touchpadDevice = {
            .device = device,
            .fd = fd,
            .size = size,
            .multiTouch = multiTouch,
            .fingerSlots = std::vector<TouchpadSlot>{slotCount}
        };
        m_devices.push_back(touchpadDevice);
    }

    if (m_devices.empty()) {
        return;
    }

    m_inputTimer.start();
}

void LibevdevComplementaryInputBackend::removeDevices()
{
    m_inputTimer.stop();
    for (auto &device : m_devices) {
        close(device.fd);
        libevdev_free(device.device);
    }
    m_devices.clear();
}

void LibevdevComplementaryInputBackend::processEvents()
{
    input_event event;
    for (auto &device : m_devices) {
        while (libevdev_has_event_pending(device.device)) {
            if (libevdev_next_event(device.device, LIBEVDEV_READ_FLAG_NORMAL, &event) != 0) {
                continue;
            }

            const auto value = event.value;
            switch (event.type) {
                case EV_SYN:
                    if (event.code == SYN_REPORT) {
                        const TouchpadSlotEvent slotEvent(device.fingerSlots);
                        handleEvent(&slotEvent);
                        continue;
                    }
                    break;
                case EV_ABS:
                    auto &currentSlot = device.fingerSlots[device.currentSlot];
                    if (device.multiTouch) {
                        switch (event.code) {
                            case ABS_MT_SLOT:
                                device.currentSlot = value;
                                break;
                            case ABS_MT_TRACKING_ID:
                                currentSlot.active = value != -1;
                                break;
                            case ABS_MT_POSITION_X:
                                currentSlot.position.setX(value / device.size.width());
                                break;
                            case ABS_MT_POSITION_Y:
                                currentSlot.position.setY(value / device.size.height());
                                break;
                            case ABS_MT_PRESSURE:
                                currentSlot.pressure = value;
                                break;
                        }
                    } else {
                        switch (event.code) {
                            case ABS_X:
                                currentSlot.position.setX(value / device.size.width());
                                break;
                            case ABS_Y:
                                currentSlot.position.setY(value / device.size.height());
                                break;
                            case ABS_PRESSURE:
                                currentSlot.pressure = value;
                                break;
                        }
                    }
                    break;
            }
        }
    }
}

}