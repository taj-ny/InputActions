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
#include <QObject>

namespace libinputactions
{

Q_LOGGING_CATEGORY(LIBINPUTACTIONS_BACKEND_LIBEVDEV, "libinputactions.input.backend.libevdev", QtWarningMsg)

LibevdevComplementaryInputBackend::LibevdevComplementaryInputBackend()
{
    m_inputTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_inputTimer.setInterval(100);
    QObject::connect(&m_inputTimer, &QTimer::timeout, [this] { poll(); });
}

LibevdevComplementaryInputBackend::~LibevdevComplementaryInputBackend()
{
    for (auto &[_, device] : m_libevdevDevices) {
        close(device.fd);
        libevdev_free(device.libevdevDevice);
    }
    m_libevdevDevices.clear();
}

void LibevdevComplementaryInputBackend::deviceAdded(InputDevice *device)
{
    InputBackend::deviceAdded(device);
    if (!m_enabled) {
        return;
    }

    if (device->type() != InputDeviceType::Touchpad) {
        return;
    }

    qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace() << "Device added (name: " << device->name() << ")";

    const auto path = QString("/dev/input/%1").arg(device->sysName()).toStdString();
    const auto fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote()
            << QString("Failed to open %1 (error %2)").arg(QString::fromStdString(path), QString::number(errno));
        return;
    }

    libevdev *libevdevDevice;
    if (libevdev_new_from_fd(fd, &libevdevDevice) != 0) {
        close(fd);
        return;
    }
    qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace() << "Opened device (name: " << libevdev_get_name(libevdevDevice) << ")";

    if (!libevdev_has_event_type(libevdevDevice, EV_ABS)) {
        qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV, "Device is not absolute");
        libevdev_free(libevdevDevice);
        close(fd);
        return;
    }

    const QSize size(libevdev_get_abs_maximum(libevdevDevice, ABS_X), libevdev_get_abs_maximum(libevdevDevice, ABS_Y));
    if (size.width() == 0 || size.height() == 0) {
        qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV, "Device has a size of 0");
        libevdev_free(libevdevDevice);
        close(fd);
        return;
    }

    bool buttonPad = libevdev_has_property(libevdevDevice, INPUT_PROP_BUTTONPAD) == 1;
    bool multiTouch{};
    uint8_t slotCount = 1;
    if (libevdev_has_event_code(libevdevDevice, EV_ABS, ABS_MT_SLOT)) {
        multiTouch = true;
        slotCount = libevdev_get_abs_maximum(libevdevDevice, ABS_MT_SLOT) + 1;
    }
    qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace()
        << "Found valid touchpad (size: " << size << ", multiTouch: " << multiTouch << ", slots: " << slotCount << ")";

    auto &properties = device->properties();
    properties.setSize(size);
    properties.setMultiTouch(multiTouch);
    properties.setButtonPad(buttonPad);

    m_libevdevDevices[device] = {
        .libevdevDevice = libevdevDevice,
        .fd = fd,
        .fingerSlots = std::vector<TouchpadSlot>{slotCount},
    };
    m_inputTimer.start();
}

void LibevdevComplementaryInputBackend::deviceRemoved(const InputDevice *device)
{
    qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace() << "Device removed (name: " << device->name() << ")";
    for (auto it = m_libevdevDevices.begin(); it != m_libevdevDevices.end(); it++) {
        if (it->first == device) {
            close(it->second.fd);
            libevdev_free(it->second.libevdevDevice);
            m_libevdevDevices.erase(it);
            break;
        }
    }

    if (m_libevdevDevices.empty()) {
        m_inputTimer.stop();
    }
}

void LibevdevComplementaryInputBackend::poll()
{
    input_event event;
    for (auto &[device, libevdevDevice] : m_libevdevDevices) {
        const auto &properties = device->properties();
        int status{};
        while (true) {
            auto flags = status == LIBEVDEV_READ_STATUS_SYNC ? LIBEVDEV_READ_FLAG_SYNC : LIBEVDEV_READ_FLAG_NORMAL;
            status = libevdev_next_event(libevdevDevice.libevdevDevice, flags, &event);
            if (status != LIBEVDEV_READ_STATUS_SUCCESS && status != LIBEVDEV_READ_STATUS_SYNC) {
                break;
            }

            const auto code = event.code;
            const auto value = event.value;
            switch (event.type) {
                case EV_SYN:
                    if (code == SYN_REPORT) {
                        const TouchpadSlotEvent slotEvent(device, libevdevDevice.fingerSlots);
                        handleEvent(&slotEvent);

                        static const std::map<uint16_t, uint8_t> fingerCountCodes = {
                            {0, 0},
                            {BTN_TOOL_FINGER, 1},
                            {BTN_TOOL_DOUBLETAP, 2},
                            {BTN_TOOL_TRIPLETAP, 3},
                            {BTN_TOOL_QUADTAP, 4},
                            {BTN_TOOL_QUINTTAP, 5}
                        };
                        VariableManager::instance()->getVariable(BuiltinVariables::Fingers)->set(fingerCountCodes.at(libevdevDevice.currentFingerCode));
                    }
                    continue;
                case EV_KEY:
                    switch (code) {
                        case BTN_TOOL_FINGER:
                        case BTN_TOOL_DOUBLETAP:
                        case BTN_TOOL_TRIPLETAP:
                        case BTN_TOOL_QUADTAP:
                        case BTN_TOOL_QUINTTAP:
                            if (value == 1) {
                                libevdevDevice.currentFingerCode = code;
                            } else if (value == 0 && libevdevDevice.currentFingerCode == code) {
                                libevdevDevice.currentFingerCode = 0;
                            }
                            continue;
                        case BTN_LEFT:
                        case BTN_MIDDLE:
                        case BTN_RIGHT:
                            if (properties.buttonPad()) {
                                const TouchpadClickEvent clickEvent(device, value);
                                handleEvent(&clickEvent);
                            }
                            continue;
                    }
                    continue;
                case EV_ABS:
                    auto &currentSlot = libevdevDevice.fingerSlots[libevdevDevice.currentSlot];
                    if (properties.multiTouch()) {
                        switch (code) {
                            case ABS_MT_SLOT:
                                libevdevDevice.currentSlot = value;
                                continue;
                            case ABS_MT_TRACKING_ID:
                                currentSlot.active = value != -1;
                                continue;
                            case ABS_MT_POSITION_X:
                                currentSlot.position.setX(value / properties.size().width());
                                continue;
                            case ABS_MT_POSITION_Y:
                                currentSlot.position.setY(value / properties.size().height());
                                continue;
                            case ABS_MT_PRESSURE:
                                currentSlot.pressure = value;
                                continue;
                        }
                    } else {
                        switch (code) {
                            case ABS_X:
                                currentSlot.position.setX(value / properties.size().width());
                                continue;
                            case ABS_Y:
                                currentSlot.position.setY(value / properties.size().height());
                                continue;
                            case ABS_PRESSURE:
                                currentSlot.pressure = value;
                                continue;
                        }
                    }
                    continue;
            }
        }
    }
}

void LibevdevComplementaryInputBackend::setPollingInterval(const uint32_t &value)
{
    m_inputTimer.setInterval(value);
}

void LibevdevComplementaryInputBackend::setEnabled(const bool &value)
{
    m_enabled = value;
}

}