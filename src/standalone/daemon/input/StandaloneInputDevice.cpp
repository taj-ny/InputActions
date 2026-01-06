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

#include "StandaloneInputDevice.h"
#include <libevdev-cpp/LibevdevDevice.h>
#include <libevdev-cpp/LibevdevUinputDevice.h>
#include <libinput-cpp/LibinputDevice.h>
#include <libinput-cpp/UdevDevice.h>
#include <libinputactions/input/backends/InputBackend.h>

namespace InputActions
{

StandaloneInputDevice::StandaloneInputDevice(InputDeviceType type, QString name, QString sysName, QString path, std::unique_ptr<LibinputPathContext> libinput,
                                             LibinputDevice *libinputDevice)
    : InputDevice(type, std::move(name), std::move(sysName))
    , m_path(std::move(path))
    , m_libinput(std::move(libinput))
    , m_libinputDevice(libinputDevice)
{
}

std::unique_ptr<StandaloneInputDevice> StandaloneInputDevice::tryCreate(const QString &path, bool &retry)
{
    auto libinput = std::make_unique<LibinputPathContext>();
    auto *libinputDevice = libinput->addDevice(path);

    if (!libinputDevice) {
        // May fail if opened before the udev rule sets ACLs, initialization will be attempted again later.
        retry = errno != ENOENT;
        return {};
    }

    const auto name = libinputDevice->name();
    const auto sysName = libinputDevice->sysName();
    InputDeviceType type{};

    const auto udevDevice = libinputDevice->udevDevice();
    if (udevDevice->propertyValue("ID_INPUT_MOUSE")) {
        type = InputDeviceType::Mouse;
    } else if (udevDevice->propertyValue("ID_INPUT_KEYBOARD")) {
        type = InputDeviceType::Keyboard;
    } else if (udevDevice->propertyValue("ID_INPUT_TOUCHPAD")) {
        type = InputDeviceType::Touchpad;
    } else {
        return {};
    }

    std::unique_ptr<StandaloneInputDevice> device(new StandaloneInputDevice(type, name, sysName, path, std::move(libinput), libinputDevice));
    const auto properties = g_inputBackend->deviceProperties(device.get());

    if (properties.ignore()) {
        return {};
    }

    if (!device->finalize(name, properties, retry)) {
        return {};
    }
    return device;
}

bool StandaloneInputDevice::finalize(const QString &name, const InputDeviceProperties &properties, bool &retry)
{
    if (properties.grab()) {
        m_libevdev = LibevdevDevice::createFromPath(m_path);
        if (!m_libevdev) {
            retry = true;
            return false;
        }

        if (!isNeutral()) {
            qWarning(INPUTACTIONS, "Failed to initialize device \"%s\": device is not in a neutral state and cannot be grabbed", name.toStdString().c_str());
            retry = true;
            return false;
        }
        m_libevdev->grab();

        m_libinputEventInjectionDevice = LibevdevUinputDevice::createManaged(m_libevdev.get(), name + " (InputActions internal)");
        if (!m_libinputEventInjectionDevice) {
            retry = true;
            return false;
        }
        m_libinputEventInjectionDevice->removeNonBlockFlag();

        m_libinput->removeDevice(m_libinputDevice);
        tryInitializeLibinputEventInjectionDevice();
        // If libinputDevice is nullptr, initialization will be reattempted later.

        m_outputDevice = LibevdevUinputDevice::createManaged(m_libevdev.get(), name + " (InputActions output)");
        if (!m_outputDevice) {
            retry = true;
            return false;
        }
    } else {
        finishLibinputDeviceInitialization();
    }

    return true;
}

uint32_t StandaloneInputDevice::tryInitializeLibinputEventInjectionDevice()
{
    m_libinputDevice = m_libinput->addDevice(m_libinputEventInjectionDevice->devNode(), true);
    if (m_libinputDevice) {
        finishLibinputDeviceInitialization();
        return 0;
    }

    return ++m_libinputEventInjectionDeviceInitializationAttempts;
}

void StandaloneInputDevice::finishLibinputDeviceInitialization()
{
    if (type() == InputDeviceType::Touchpad) {
        m_libinputDevice->configTapSetEnabled(true);
    }
}

bool StandaloneInputDevice::isNeutral() const
{
    switch (type()) {
        case InputDeviceType::Keyboard:
        case InputDeviceType::Mouse:
            for (int code = 0; code < KEY_MAX; code++) {
                if (m_libevdev->hasEventCode(EV_KEY, code) && m_libevdev->eventValue(EV_KEY, code)) {
                    return false;
                }
            }
            break;
        case InputDeviceType::Touchpad:
            return !m_libevdev->hasEventCode(EV_KEY, BTN_TOUCH) || !m_libevdev->eventValue(EV_KEY, BTN_TOUCH);
    }
    return true;
}

bool StandaloneInputDevice::isDeviceOwnedByThisDevice(const QString &path) const
{
    return (m_libinputEventInjectionDevice && path == m_libinputEventInjectionDevice->devNode()) || (m_outputDevice && path == m_outputDevice->devNode());
}

void StandaloneInputDevice::resetVirtualDeviceState()
{
    switch (type()) {
        case InputDeviceType::Keyboard:
        case InputDeviceType::Mouse: {
            bool syn{};
            for (int code = 0; code < KEY_MAX; code++) {
                if (m_libevdev->hasEventCode(EV_KEY, code) && m_libevdev->eventValue(EV_KEY, code)) {
                    syn = true;
                    m_outputDevice->writeEvent(EV_KEY, code, 0);
                }
            }

            if (syn) {
                m_outputDevice->writeSynReportEvent();
            }
            break;
        }
        case InputDeviceType::Touchpad:
            // Reverse order so that ABS_MT_SLOT is equal to 0 after
            for (int i = touchPoints().size() - 1; i >= 0; i--) {
                m_outputDevice->writeEvent(EV_ABS, ABS_MT_SLOT, i);
                m_outputDevice->writeEvent(EV_ABS, ABS_MT_TRACKING_ID, -1);
            }

            m_outputDevice->writeEvent(EV_KEY, BTN_TOOL_QUINTTAP, 0);
            m_outputDevice->writeEvent(EV_KEY, BTN_TOOL_QUADTAP, 0);
            m_outputDevice->writeEvent(EV_KEY, BTN_TOOL_TRIPLETAP, 0);
            m_outputDevice->writeEvent(EV_KEY, BTN_TOUCH, 0);
            m_outputDevice->writeEvent(EV_KEY, BTN_TOOL_DOUBLETAP, 0);
            m_outputDevice->writeEvent(EV_KEY, BTN_TOOL_FINGER, 0);
            m_outputDevice->writeEvent(EV_ABS, ABS_PRESSURE, 0);
            m_outputDevice->writeSynReportEvent();
            break;
    }
}

void StandaloneInputDevice::restoreVirtualDeviceState()
{
    switch (type()) {
        case InputDeviceType::Touchpad:
            for (int code = 0; code < KEY_MAX; code++) {
                if (!m_libevdev->hasEventCode(EV_KEY, code)) {
                    continue;
                }

                m_outputDevice->writeEvent(EV_KEY, code, m_libevdev->eventValue(EV_KEY, code));
            }

            for (int code = 0; code < ABS_MAX; code++) {
                if ((code >= ABS_MT_SLOT && code <= ABS_MT_TOOL_Y) || !m_libevdev->hasEventCode(EV_ABS, code)) {
                    continue;
                }

                m_outputDevice->writeEvent(EV_ABS, code, m_libevdev->absInfo(code)->value);
            }

            for (int slot = 0; slot < m_libevdev->slotCount(); slot++) {
                m_outputDevice->writeEvent(EV_ABS, ABS_MT_SLOT, slot);

                for (int code = ABS_MT_SLOT; code <= ABS_MT_TOOL_Y; code++) {
                    if (code == ABS_MT_SLOT || !m_libevdev->hasEventCode(EV_ABS, code)) {
                        continue;
                    }

                    m_outputDevice->writeEvent(EV_ABS, code, m_libevdev->slotValue(slot, code));
                }
            }

            m_outputDevice->writeEvent(EV_ABS, ABS_MT_SLOT, m_libevdev->currentSlot());
            m_outputDevice->writeSynReportEvent();
            break;
    }
}

}