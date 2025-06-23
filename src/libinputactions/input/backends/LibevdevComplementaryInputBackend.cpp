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

#include "LibevdevComplementaryInputBackend.h"

#include <libinputactions/interfaces/SessionLock.h>
#include <libinputactions/variables/VariableManager.h>

#include <fcntl.h>

#include <QDir>
#include <QLoggingCategory>
#include <QObject>

namespace libinputactions
{

Q_LOGGING_CATEGORY(INPUTACTIONS_BACKEND_LIBEVDEV, "inputactions.input.backend.libevdev", QtWarningMsg)

LibevdevComplementaryInputBackend::LibevdevComplementaryInputBackend()
{
    m_inputTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_inputTimer.setInterval(100);
    QObject::connect(&m_inputTimer, &QTimer::timeout, [this] { poll(); });
}

LibevdevDevice::~LibevdevDevice()
{
    if (libevdevPtr) {
        libevdev_free(libevdevPtr);
    }
    if (fd != -1) {
        close(fd);
    }
}

std::unique_ptr<LibevdevDevice> LibevdevComplementaryInputBackend::openDevice(const QString &sysName)
{
    auto device = std::make_unique<LibevdevDevice>();
    const auto path = QString("/dev/input/%1").arg(sysName).toStdString();
    device->fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (device->fd == -1) {
        qCDebug(INPUTACTIONS_BACKEND_LIBEVDEV).noquote()
                    << QString("Failed to open %1 (error %2)").arg(QString::fromStdString(path), QString::number(errno));
        return {};
    }

    if (libevdev_new_from_fd(device->fd, &device->libevdevPtr) != 0) {
        device->libevdevPtr = nullptr;
        return {};
    }
    device->name = QString::fromStdString(libevdev_get_name(device->libevdevPtr));
    return device;
    qCDebug(INPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace() << "Opened device (name: " << libevdev_get_name(device->libevdevPtr) << ")";
}

void LibevdevComplementaryInputBackend::deviceAdded(InputDevice *device)
{
    InputBackend::deviceAdded(device);
    if (!m_enabled) {
        return;
    }
    qCDebug(INPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace() << "Device added (name: " << device->name() << ")";

    if (device->type() != InputDeviceType::Touchpad) {
        return;
    }

    std::unique_ptr<LibevdevDevice> libevdevDevice;
    if (!device->sysName().isEmpty()) {
        libevdevDevice = openDevice(device->sysName());
    } else {
        // If sysName is not available, go through all devices and find one with the same name
        for (const auto &entry : QDir("/dev/input").entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::System)) {
            libevdevDevice = openDevice(entry.fileName());
            if (!libevdevDevice) {
                continue;
            }

            if (libevdevDevice->name == device->name()) {
                break;
            }
            libevdevDevice.reset();
        }
    }
    if (!libevdevDevice) {
        return;
    }

    if (!libevdev_has_event_type(libevdevDevice->libevdevPtr, EV_ABS)) {
        qCDebug(INPUTACTIONS_BACKEND_LIBEVDEV, "Device is not absolute");
        return;
    }

    const QSize size(libevdev_get_abs_maximum(libevdevDevice->libevdevPtr, ABS_X), libevdev_get_abs_maximum(libevdevDevice->libevdevPtr, ABS_Y));
    if (size.width() == 0 || size.height() == 0) {
        qCDebug(INPUTACTIONS_BACKEND_LIBEVDEV, "Device has a size of 0");
        return;
    }

    bool buttonPad = libevdev_has_property(libevdevDevice->libevdevPtr, INPUT_PROP_BUTTONPAD) == 1;
    bool multiTouch{};
    uint8_t slotCount = 1;
    if (libevdev_has_event_code(libevdevDevice->libevdevPtr, EV_ABS, ABS_MT_SLOT)) {
        multiTouch = true;
        slotCount = libevdev_get_abs_maximum(libevdevDevice->libevdevPtr, ABS_MT_SLOT) + 1;
    }
    qCDebug(INPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace()
        << "Found valid touchpad (size: " << size << ", multiTouch: " << multiTouch << ", slots: " << slotCount << ")";

    auto &properties = device->properties();
    properties.setSize(size);
    properties.setMultiTouch(multiTouch);
    properties.setButtonPad(buttonPad);

    libevdevDevice->fingerSlots = std::vector<TouchpadSlot>{slotCount};
    m_libevdevDevices[device] = std::move(libevdevDevice);
    m_inputTimer.start();
}

void LibevdevComplementaryInputBackend::deviceRemoved(const InputDevice *device)
{
    qCDebug(INPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace() << "Device removed (name: " << device->name() << ")";
    for (auto it = m_libevdevDevices.begin(); it != m_libevdevDevices.end(); it++) {
        if (it->first == device) {
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
    if (m_ignoreEvents || SessionLock::instance()->sessionLocked()) {
        return;
    }

    input_event event;
    for (auto &[device, libevdevDevice] : m_libevdevDevices) {
        const auto &properties = device->properties();
        int status{};
        while (true) {
            auto flags = status == LIBEVDEV_READ_STATUS_SYNC ? LIBEVDEV_READ_FLAG_SYNC : LIBEVDEV_READ_FLAG_NORMAL;
            status = libevdev_next_event(libevdevDevice->libevdevPtr, flags, &event);
            if (status != LIBEVDEV_READ_STATUS_SUCCESS && status != LIBEVDEV_READ_STATUS_SYNC) {
                break;
            }

            const auto code = event.code;
            const auto value = event.value;
            switch (event.type) {
                case EV_SYN:
                    if (code == SYN_REPORT) {
                        const TouchpadSlotEvent slotEvent(device, libevdevDevice->fingerSlots);
                        handleEvent(&slotEvent);

                        static const std::map<uint16_t, uint8_t> fingerCountCodes = {
                            {0, 0},
                            {BTN_TOOL_FINGER, 1},
                            {BTN_TOOL_DOUBLETAP, 2},
                            {BTN_TOOL_TRIPLETAP, 3},
                            {BTN_TOOL_QUADTAP, 4},
                            {BTN_TOOL_QUINTTAP, 5}
                        };
                        VariableManager::instance()->getVariable(BuiltinVariables::Fingers)->set(fingerCountCodes.at(libevdevDevice->currentFingerCode));
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
                                libevdevDevice->currentFingerCode = code;
                            } else if (value == 0 && libevdevDevice->currentFingerCode == code) {
                                libevdevDevice->currentFingerCode = 0;
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
                    auto &currentSlot = libevdevDevice->fingerSlots[libevdevDevice->currentSlot];
                    if (properties.multiTouch()) {
                        switch (code) {
                            case ABS_MT_SLOT:
                                libevdevDevice->currentSlot = value;
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