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
#include <QDir>
#include <QLoggingCategory>
#include <QObject>
#include <experimental/scope>
#include <fcntl.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

Q_LOGGING_CATEGORY(INPUTACTIONS_BACKEND_LIBEVDEV, "inputactions.input.backend.libevdev", QtWarningMsg)

LibevdevComplementaryInputBackend::LibevdevComplementaryInputBackend()
{
    m_inputTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_inputTimer.setInterval(10);
    QObject::connect(&m_inputTimer, &QTimer::timeout, [this] {
        poll();
    });
}

libevdev *LibevdevComplementaryInputBackend::openDevice(const QString &sysName)
{
    const auto path = QString("/dev/input/%1").arg(sysName).toStdString();
    const auto fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        qCDebug(INPUTACTIONS_BACKEND_LIBEVDEV).noquote() << QString("Failed to open %1 (error %2)").arg(QString::fromStdString(path), QString::number(errno));
        return {};
    }
    fcntl(fd, F_SETFD, FD_CLOEXEC);

    libevdev *libevdev{};
    if (libevdev_new_from_fd(fd, &libevdev) == 0) {
        return libevdev;
    }
    return nullptr;
}

void LibevdevComplementaryInputBackend::addDevice(InputDevice *device, libevdev *libevdev, bool owner)
{
    auto data = std::make_unique<ExtraDeviceData>();
    data->libevdev = libevdev;
    data->owner = owner;

    if (!libevdev_has_event_type(libevdev, EV_ABS)) {
        qCDebug(INPUTACTIONS_BACKEND_LIBEVDEV, "Device is not absolute");
        return;
    }

    const auto *x = libevdev_get_abs_info(libevdev, ABS_X);
    const auto *y = libevdev_get_abs_info(libevdev, ABS_Y);
    if (!x || !y) {
        return;
    }

    data->absMin = {std::abs(x->minimum), std::abs(y->minimum)};
    const QSize size(data->absMin.x() + x->maximum, data->absMin.y() + y->maximum);
    if (size.width() == 0 || size.height() == 0) {
        qCDebug(INPUTACTIONS_BACKEND_LIBEVDEV, "Device has a size of 0");
        return;
    }

    bool buttonPad = libevdev_has_property(libevdev, INPUT_PROP_BUTTONPAD) == 1;
    bool multiTouch{};
    uint8_t slotCount = 1;
    if (libevdev_has_event_code(libevdev, EV_ABS, ABS_MT_SLOT)) {
        multiTouch = true;
        slotCount = libevdev_get_abs_maximum(libevdev, ABS_MT_SLOT) + 1;
    }
    device->m_touchPoints = std::vector<TouchPoint>(slotCount);
    qCDebug(INPUTACTIONS_BACKEND_LIBEVDEV).noquote().nospace()
        << "Found valid touchpad (size: " << size << ", multiTouch: " << multiTouch << ", slots: " << slotCount << ")";

    auto &properties = device->properties();
    properties.setSize(size);
    properties.setMultiTouch(multiTouch);
    properties.setButtonPad(buttonPad);

    m_devices[device] = std::move(data);
}

void LibevdevComplementaryInputBackend::deviceAdded(InputDevice *device)
{
    std::experimental::scope_exit guard([this, device]() {
        InputBackend::deviceAdded(device);
    });

    if (!m_enabled || device->type() != InputDeviceType::Touchpad || !deviceProperties(device).handleLibevdevEvents()) {
        return;
    }

    libevdev *libevdev{};
    if (!device->sysName().isEmpty()) {
        libevdev = openDevice(device->sysName());
    } else {
        // If sysName is not available, go through all devices and find one with the same name
        for (const auto &entry : QDir("/dev/input").entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::System)) {
            libevdev = openDevice(entry.fileName());
            if (!libevdev) {
                continue;
            }

            if (libevdev_get_name(libevdev) == device->name()) {
                break;
            }
        }
    }
    if (!libevdev) {
        return;
    }

    addDevice(device, libevdev, true);
    m_inputTimer.start();
}

void LibevdevComplementaryInputBackend::deviceRemoved(const InputDevice *device)
{
    InputBackend::deviceRemoved(device);
    m_devices.erase(const_cast<InputDevice *>(device));
    if (m_devices.empty()) {
        m_inputTimer.stop();
    }
}

void LibevdevComplementaryInputBackend::handleEvdevEvent(InputDevice *sender, const input_event &event)
{
    if (!m_devices.contains(sender)) {
        return;
    }

    auto &data = m_devices[sender];
    const auto &properties = sender->properties();

    const auto code = event.code;
    const auto value = event.value;
    switch (event.type) {
        case EV_SYN:
            if (code == SYN_REPORT) {
                for (size_t i = 0; i < data->previousTouchPoints.size(); i++) {
                    const auto &previous = data->previousTouchPoints[i];
                    auto &slot = sender->m_touchPoints[i];

                    if (slot.pressure >= properties.palmPressure()) {
                        slot.type = TouchPointType::Palm;
                    } else if (slot.pressure >= properties.thumbPressure()) {
                        slot.type = TouchPointType::Thumb;
                    } else if (slot.pressure >= properties.fingerPressure()) {
                        slot.type = TouchPointType::Finger;
                    } else {
                        slot.type = TouchPointType::None;
                    }
                    slot.valid = slot.active && (slot.type == TouchPointType::Finger || slot.type == TouchPointType::Thumb);

                    if (previous.valid != slot.valid) {
                        if (slot.valid) {
                            slot.downTimestamp = std::chrono::steady_clock::now();
                            slot.initialPosition = slot.position;
                        }

                        handleEvent(TouchEvent(sender, slot.valid ? InputEventType::TouchDown : InputEventType::TouchUp, slot));
                    } else if (previous.position != slot.position || previous.pressure != slot.pressure) {
                        handleEvent(TouchChangedEvent(sender, slot, slot.position - previous.position));
                    }
                }
                data->previousTouchPoints = sender->m_touchPoints;
            }
            break;
        case EV_KEY:
            switch (code) {
                case BTN_LEFT:
                case BTN_MIDDLE:
                case BTN_RIGHT:
                    if (properties.buttonPad()) {
                        handleEvent(TouchpadClickEvent(sender, value));
                    }
                    break;
            }
            break;
        case EV_ABS:
            auto &currentTouchPoint = sender->m_touchPoints[data->currentSlot];
            if (properties.multiTouch()) {
                switch (code) {
                    case ABS_MT_SLOT:
                        data->currentSlot = value;
                        break;
                    case ABS_MT_TRACKING_ID:
                        currentTouchPoint.active = value != -1;
                        break;
                    case ABS_MT_POSITION_X:
                        currentTouchPoint.position.setX((value + data->absMin.x()) / properties.size().width());
                        break;
                    case ABS_MT_POSITION_Y:
                        currentTouchPoint.position.setY((value + data->absMin.y()) / properties.size().height());
                        break;
                    case ABS_MT_PRESSURE:
                        currentTouchPoint.pressure = value;
                        break;
                }
            } else {
                switch (code) {
                    case ABS_X:
                        currentTouchPoint.position.setX((value + data->absMin.x()) / properties.size().width());
                        break;
                    case ABS_Y:
                        currentTouchPoint.position.setY((value + data->absMin.y()) / properties.size().height());
                        break;
                    case ABS_PRESSURE:
                        currentTouchPoint.pressure = value;
                        break;
                }
            }
            break;
    }
}

void LibevdevComplementaryInputBackend::poll()
{
    if (m_ignoreEvents) {
        return;
    }

    input_event event;
    for (auto &[device, data] : m_devices) {
        if (!data->owner || !data->libevdev) {
            continue;
        }

        int status{};
        while (true) {
            auto flags = status == LIBEVDEV_READ_STATUS_SYNC ? LIBEVDEV_READ_FLAG_SYNC : LIBEVDEV_READ_FLAG_NORMAL;
            status = libevdev_next_event(data->libevdev, flags, &event);
            if (status != LIBEVDEV_READ_STATUS_SUCCESS && status != LIBEVDEV_READ_STATUS_SYNC) {
                break;
            }

            handleEvdevEvent(device, event);
        }
    }
}

void LibevdevComplementaryInputBackend::setPollingInterval(uint32_t value)
{
    m_inputTimer.setInterval(value);
}

void LibevdevComplementaryInputBackend::setEnabled(bool value)
{
    m_enabled = value;
}

LibevdevComplementaryInputBackend::ExtraDeviceData::~ExtraDeviceData()
{
    if (owner && libevdev) {
        close(libevdev_get_fd(libevdev));
        libevdev_free(libevdev);
    }
}

}
