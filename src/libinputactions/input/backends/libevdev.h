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

#include <libinputactions/input/backends/backend.h>

#include <libevdev-1.0/libevdev/libevdev.h>

#include <set>
#include <thread>
#include <vector>

#include <QFileSystemWatcher>
#include <QPoint>
#include <QTimer>
#include <QSize>

namespace libinputactions
{

struct TouchpadDevice
{
    QString devInputName;
    libevdev *device;
    int fd;

    QSizeF size;
    bool multiTouch;
    bool buttonPad{};

    /**
     * If device doesn't support MT type B protocol, only the first slot will be used.
     */
    std::vector<TouchpadSlot> fingerSlots;
    uint8_t currentSlot{};
    /**
     * 0, BTN_TOOL_FINGER, BTN_TOOL_DOUBLETAP, BTN_TOOL_TRIPLETAP, BTN_TOOL_QUADTAP or BTN_TOOL_QUINTTAP
     */
    uint16_t currentFingerCode{};
};

/**
 * Uses libevdev to get additional touchpad data that libinput does not provide.
 *
 * Emitted events: TouchpadClick, TouchpadSlot
 */
class LibevdevComplementaryInputBackend : public virtual InputBackend
{
public:
    LibevdevComplementaryInputBackend();
    ~LibevdevComplementaryInputBackend();

    void poll() override;

    /**
     * @param value How often to poll input events. A too high value may result in missed events.
     */
    void setPollingInterval(const uint32_t &value);

private:
    void devInputChanged();
    void deviceAdded(const QString &name);
    void deviceRemoved(const QString &name);
    /**
     * @return Names of all devices in /dev/input matching "event*".
     */
    QList<QString> devInputDevices() const;

    std::vector<TouchpadDevice> m_devices;
    QTimer m_inputTimer;

    QFileSystemWatcher m_devInputWatcher;
    std::set<QString> m_devInputDevices;
};
}