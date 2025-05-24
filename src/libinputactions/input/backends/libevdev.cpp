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

    m_devInputWatcher.addPath("/dev/input");
    QObject::connect(&m_devInputWatcher, &QFileSystemWatcher::directoryChanged, [this] { devInputChanged(); });
    devInputChanged();
}

LibevdevComplementaryInputBackend::~LibevdevComplementaryInputBackend()
{
    for (auto &device : m_devices) {
        close(device.fd);
        libevdev_free(device.device);
    }
    m_devices.clear();
}

void LibevdevComplementaryInputBackend::devInputChanged()
{
    const auto devInput = devInputDevices();
    for (const auto &device : m_devInputDevices) {
        if (!devInput.contains(device)) {
            deviceRemoved(device);
        }
    }
    for (const auto &device : devInput) {
        if (!m_devInputDevices.contains(device)) {
            deviceAdded(device);
        }
    }
}

void LibevdevComplementaryInputBackend::deviceAdded(const QString &name)
{
    qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace() << "Device added (name: " << name << ")";
    m_devInputDevices.insert(name);

    const auto path = QString("/dev/input/%1").arg(name).toStdString();
    const auto fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote()
            << QString("Failed to open %1 (error %2)").arg(QString::fromStdString(path), QString::number(errno));
        return;
    }

    libevdev *device;
    if (libevdev_new_from_fd(fd, &device) != 0) {
        close(fd);
        return;
    }
    qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace() << "Opened device (name: " << libevdev_get_name(device) << ")";

    if (!libevdev_has_event_type(device, EV_ABS)) {
        qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV, "Device is not absolute");
        libevdev_free(device);
        close(fd);
        return;
    }

    const QSize size(libevdev_get_abs_maximum(device, ABS_X), libevdev_get_abs_maximum(device, ABS_Y));
    if (size.width() == 0 || size.height() == 0) {
        qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV, "Device has a size of 0");
        libevdev_free(device);
        close(fd);
        return;
    }

    bool multiTouch{};
    uint8_t slotCount = 1;
    if (libevdev_has_event_code(device, EV_ABS, ABS_MT_SLOT)) {
        multiTouch = true;
        slotCount = libevdev_get_abs_maximum(device, ABS_MT_SLOT) + 1;
    }
    qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace()
        << "Found valid touchpad (size: " << size << ", multiTouch: " << multiTouch << ", slots: " << slotCount << ")";
    TouchpadDevice touchpadDevice = {
        .devInputName = name,
        .device = device,
        .fd = fd,
        .size = size,
        .multiTouch = multiTouch,
        .buttonPad = libevdev_has_property(device, INPUT_PROP_BUTTONPAD) == 1,
        .fingerSlots = std::vector<TouchpadSlot>{slotCount}
    };
    m_devices.push_back(touchpadDevice);
    m_inputTimer.start();
}

void LibevdevComplementaryInputBackend::deviceRemoved(const QString &name)
{
    qCDebug(LIBINPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace() << "Device removed (name: " << name << ")";
    m_devInputDevices.erase(name);
    for (auto it = m_devices.begin(); it != m_devices.end(); it++) {
        if (it->devInputName == name) {
            close(it->fd);
            libevdev_free(it->device);
            m_devices.erase(it);
            break;
        }
    }

    if (m_devices.empty()) {
        m_inputTimer.stop();
    }
}

QList<QString> LibevdevComplementaryInputBackend::devInputDevices() const
{
    return QDir("/dev/input").entryList(QDir::Filter::System).filter("event");
}

void LibevdevComplementaryInputBackend::poll()
{
    input_event event;
    for (auto &device : m_devices) {
        while (libevdev_has_event_pending(device.device)) {
            // This doesn't handle LIBEVDEV_READ_STATUS_SYNC
            if (libevdev_next_event(device.device, LIBEVDEV_READ_FLAG_NORMAL, &event) != LIBEVDEV_READ_STATUS_SUCCESS) {
                continue;
            }

            const auto code = event.code;
            const auto value = event.value;
            switch (event.type) {
                case EV_SYN:
                    if (code == SYN_REPORT) {
                        const TouchpadSlotEvent slotEvent(InputDevice(InputDeviceType::Touchpad), device.fingerSlots);
                        handleEvent(&slotEvent);

                        static const std::map<uint16_t, uint8_t> fingerCountCodes = {
                            {0, 0},
                            {BTN_TOOL_FINGER, 1},
                            {BTN_TOOL_DOUBLETAP, 2},
                            {BTN_TOOL_TRIPLETAP, 3},
                            {BTN_TOOL_QUADTAP, 4},
                            {BTN_TOOL_QUINTTAP, 5}
                        };
                        VariableManager::instance()->getVariable(BuiltinVariables::Fingers)->set(fingerCountCodes.at(device.currentFingerCode));
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
                                device.currentFingerCode = code;
                            } else if (value == 0 && device.currentFingerCode == code) {
                                device.currentFingerCode = 0;
                            }
                            continue;
                        case BTN_LEFT:
                        case BTN_MIDDLE:
                        case BTN_RIGHT:
                            if (device.buttonPad) {
                                const TouchpadClickEvent clickEvent(InputDevice(InputDeviceType::Touchpad), value);
                                handleEvent(&clickEvent);
                            }
                            continue;
                    }
                    continue;
                case EV_ABS:
                    auto &currentSlot = device.fingerSlots[device.currentSlot];
                    if (device.multiTouch) {
                        switch (code) {
                            case ABS_MT_SLOT:
                                device.currentSlot = value;
                                continue;
                            case ABS_MT_TRACKING_ID:
                                currentSlot.active = value != -1;
                                continue;
                            case ABS_MT_POSITION_X:
                                currentSlot.position.setX(value / device.size.width());
                                continue;
                            case ABS_MT_POSITION_Y:
                                currentSlot.position.setY(value / device.size.height());
                                continue;
                            case ABS_MT_PRESSURE:
                                currentSlot.pressure = value;
                                continue;
                        }
                    } else {
                        switch (code) {
                            case ABS_X:
                                currentSlot.position.setX(value / device.size.width());
                                continue;
                            case ABS_Y:
                                currentSlot.position.setY(value / device.size.height());
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

}