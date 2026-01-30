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

#include "LibevdevComplementaryInputBackend.h"
#include <QDir>
#include <QObject>
#include <fcntl.h>
#include <libevdev-cpp/LibevdevDevice.h>
#include <libevdev/libevdev.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/input/devices/InputDeviceProperties.h>
#include <libinputactions/input/events.h>
#include <libinputactions/variables/VariableManager.h>

namespace InputActions
{

void LibevdevComplementaryInputBackend::addDevice(InputDevice *device)
{
    if (!m_enabled || device->type() != InputDeviceType::Touchpad) {
        return;
    }

    std::expected<std::unique_ptr<LibevdevDevice>, int> libevdevDevice;
    if (!device->sysName().isEmpty()) {
        libevdevDevice = LibevdevDevice::createFromPath("/dev/input/" + device->sysName());
    } else {
        // If sysName is not available, go through all devices and find one with the same name
        for (const auto &entry : QDir("/dev/input").entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::System)) {
            auto candidate = LibevdevDevice::createFromPath(entry.filePath());
            if (!candidate || candidate.value()->name() != device->name()) {
                continue;
            }

            libevdevDevice = std::move(candidate);
            break;
        }
    }
    if (!libevdevDevice) {
        return;
    }

    addDevice(device, std::move(libevdevDevice.value()), true);
}

void LibevdevComplementaryInputBackend::addDevice(InputDevice *device, std::shared_ptr<LibevdevDevice> libevdevDevice)
{
    if (!m_enabled || device->type() != InputDeviceType::Touchpad) {
        return;
    }

    addDevice(device, std::move(libevdevDevice), false);
}

void LibevdevComplementaryInputBackend::addDevice(InputDevice *device, std::shared_ptr<LibevdevDevice> libevdevDevice, bool owner)
{
    auto data = std::make_unique<ExtraDeviceData>();
    data->device = libevdevDevice;
    data->owner = owner;

    if (!libevdevDevice->hasEventType(EV_ABS)) {
        return;
    }

    const auto *x = libevdevDevice->absInfo(ABS_X);
    const auto *y = libevdevDevice->absInfo(ABS_Y);
    if (!x || !y) {
        return;
    }

    data->absMin = {std::abs(x->minimum), std::abs(y->minimum)};
    const QSize size(data->absMin.x() + x->maximum, data->absMin.y() + y->maximum);
    if (size.width() == 0 || size.height() == 0) {
        return;
    }

    bool buttonPad = libevdevDevice->hasProperty(INPUT_PROP_BUTTONPAD);
    bool multiTouch{};
    if (libevdevDevice->hasEventCode(EV_ABS, ABS_MT_SLOT)) {
        multiTouch = true;
    }

    auto &properties = device->properties();
    properties.setSize(size);
    properties.setMultiTouch(multiTouch);
    properties.setButtonPad(buttonPad);

    if (owner) {
        connect(data->device.get(), &LibevdevDevice::eventsAvailable, this, [this, device] {
            poll(device);
        });
    }

    m_devices[device] = std::move(data);
}

void LibevdevComplementaryInputBackend::removeDevice(const InputDevice *device)
{
    m_devices.erase(const_cast<InputDevice *>(device));
    InputBackend::removeDevice(device);
}

void LibevdevComplementaryInputBackend::handleEvdevEvent(InputDevice *sender, const input_event &event)
{
    if (!m_devices.contains(sender) || !sender->properties().handleLibevdevEvents()) {
        return;
    }

    auto &data = m_devices[sender];
    const auto &properties = sender->properties();

    const auto code = event.code;
    const auto value = event.value;
    switch (event.type) {
        case EV_SYN:
            if (code == SYN_REPORT) {
                for (const auto &[id, slot] : data->currentSlots) {
                    const auto previousSlot = data->previousSlots[id];

                    if (previousSlot.active && !slot.active) {
                        handleEvent(TouchUpEvent(sender, id));
                        continue;
                    }
                    if (!previousSlot.active && slot.active) {
                        handleEvent(TouchDownEvent(sender, id, slot.position, slot.position, slot.pressure));
                        continue;
                    }
                    if (previousSlot.position != slot.position) {
                        handleEvent(TouchMotionEvent(sender, id, slot.position, slot.position));
                    }
                    if (previousSlot.pressure != slot.pressure) {
                        handleEvent(TouchPressureChangeEvent(sender, id, slot.pressure));
                    }
                }
                data->previousSlots = data->currentSlots;
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
            auto &slot = data->currentSlots[data->currentSlot];
            if (properties.multiTouch()) {
                switch (code) {
                    case ABS_MT_SLOT:
                        data->currentSlot = value;
                        break;
                    case ABS_MT_TRACKING_ID:
                        slot.active = value != -1;
                        break;
                    case ABS_MT_POSITION_X:
                        slot.position.setX(value + data->absMin.x());
                        break;
                    case ABS_MT_POSITION_Y:
                        slot.position.setY(value + data->absMin.y());
                        break;
                    case ABS_MT_PRESSURE:
                        slot.pressure = value;
                        break;
                }
            } else {
                switch (code) {
                    case ABS_X:
                        slot.position.setX(value + data->absMin.x());
                        break;
                    case ABS_Y:
                        slot.position.setY(value + data->absMin.y());
                        break;
                    case ABS_PRESSURE:
                        slot.pressure = value;
                        break;
                }
            }
            break;
    }
}

void LibevdevComplementaryInputBackend::poll(InputDevice *device)
{
    if (m_ignoreEvents || !m_devices.contains(device)) {
        return;
    }

    const auto &data = m_devices[device];
    if (!data->owner || !data->device) {
        return;
    }

    input_event event;
    int status{};
    while (true) {
        auto flags = status == LIBEVDEV_READ_STATUS_SYNC ? LIBEVDEV_READ_FLAG_SYNC : LIBEVDEV_READ_FLAG_NORMAL;
        status = data->device->nextEvent(flags, event);
        if (status != LIBEVDEV_READ_STATUS_SUCCESS && status != LIBEVDEV_READ_STATUS_SYNC) {
            break;
        }

        handleEvdevEvent(device, event);
    }
}

void LibevdevComplementaryInputBackend::setEnabled(bool value)
{
    m_enabled = value;
}

LibevdevComplementaryInputBackend::ExtraDeviceData::ExtraDeviceData() = default;
LibevdevComplementaryInputBackend::ExtraDeviceData::~ExtraDeviceData() = default;

}
