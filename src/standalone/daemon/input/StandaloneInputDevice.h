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

#include <QTimer>
#include <libevdev-cpp/LibevdevUinputDevice.h>
#include <libinput-cpp/LibinputPathContext.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <optional>

namespace InputActions
{

class LibevdevDevice;
class LibevdevUinputDevice;
class LibinputDevice;

class StandaloneInputDevice : public InputDevice
{
public:
    Q_DISABLE_COPY_MOVE(StandaloneInputDevice);

    static std::unique_ptr<StandaloneInputDevice> tryCreate(const QString &path, bool &retry);

    const QString &path() const { return m_path; }

    /**
     * The physical device. Nullptr if the device is not grabbed.
     */
    const std::shared_ptr<LibevdevDevice> &libevdev() { return m_libevdev; }

    /**
     * Libinput context containing only libinputDevice.
     */
    LibinputPathContext *libinput() { return m_libinput.get(); }
    /**
     * If grabbed, this is the libinput event injection device, otherwise it is the physical one.
     */
    LibinputDevice *libinputDevice() { return m_libinputDevice; }
    /**
     * The virtual device for injecting evdev events into libinput, as there is no API for that. Grabbed by libinput. Nullptr if the device is not grabbed.
     */
    LibevdevUinputDevice *libinputEventInjectionDevice() { return m_libinputEventInjectionDevice ? &m_libinputEventInjectionDevice.value() : nullptr; }

    /**
     * The virtual device where non-filtered and simulated events are written to be later processed by the compositor. Nullptr if the device is not grabbed.
     */
    LibevdevUinputDevice *outputDevice() { return m_outputDevice ? &m_outputDevice.value() : nullptr; }

    bool isTouchpadBlocked() const { return m_touchpadBlocked; }
    void setTouchpadBlocked(bool value) { m_touchpadBlocked = value; }

    bool isTouchpadNeutral() const { return m_touchpadNeutral; }
    void setTouchpadNeutral(bool value) { m_touchpadNeutral = value; }

    QTimer &touchpadStateResetTimer() { return m_touchpadStateResetTimer; }

    /**
     * @return 0 on success, otherwise the amount of total initialization attempts
     */
    uint32_t tryInitializeLibinputEventInjectionDevice();
    bool isLibinputEventInjectionDeviceInitialized() const { return m_libinputDevice; }

    /**
     * @return Whether the device at the specified path is a virtual device created by this device.
     */
    bool isDeviceOwnedByThisDevice(const QString &path) const;

    void mouseButton(uint32_t button, bool state) override;
    void keyboardKey(uint32_t key, bool state) override;

    void resetVirtualDeviceState() override;
    void restoreVirtualDeviceState() override;

protected:
    void touchscreenTapDown(const std::vector<QPointF> &points) override;
    void touchscreenTapUp(const std::vector<QPointF> &points) override;

private:
    StandaloneInputDevice(InputDeviceType type, QString name, QString sysName, QString path, std::unique_ptr<LibinputPathContext> libinput,
                          LibinputDevice *libinputDevice);

    bool finalize(const QString &name, const InputDeviceProperties &properties, bool &retry);
    void finishLibinputDeviceInitialization();

    /**
     * @return Whether the device is in a neutral state. For touchpads use isTouchpadNeutral once event processing begins.
     */
    bool isNeutral() const;

    QString m_path;

    std::shared_ptr<LibevdevDevice> m_libevdev;

    std::unique_ptr<LibinputPathContext> m_libinput;
    LibinputDevice *m_libinputDevice{};

    std::optional<LibevdevUinputDevice> m_libinputEventInjectionDevice;
    uint32_t m_libinputEventInjectionDeviceInitializationAttempts{};

    std::optional<LibevdevUinputDevice> m_outputDevice;

    bool m_touchpadBlocked{};
    bool m_touchpadNeutral{};
    QTimer m_touchpadStateResetTimer;
};

}