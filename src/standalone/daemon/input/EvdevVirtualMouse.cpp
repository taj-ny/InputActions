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

#include "EvdevVirtualMouse.h"
#include <libevdev-cpp/Device.h>
#include <libevdev-cpp/exceptions.h>

namespace InputActions
{

EvdevVirtualMouse::EvdevVirtualMouse()
{
    libevdev::Device device;
    device.enableEventType(EV_KEY);
    for (uint32_t button = BTN_LEFT; button < BTN_JOYSTICK; button++) {
        device.enableEventCode(EV_KEY, button, nullptr);
    }

    device.enableEventType(EV_REL);
    device.enableEventCode(EV_REL, REL_X, nullptr);
    device.enableEventCode(EV_REL, REL_Y, nullptr);
    device.enableEventCode(EV_REL, REL_WHEEL_HI_RES, nullptr);
    device.enableEventCode(EV_REL, REL_HWHEEL_HI_RES, nullptr);

    try {
        m_device = libevdev::UInputDevice::createManaged(&device, "InputActions Virtual Mouse");
    } catch (const libevdev::Exception &) {
    }
}

EvdevVirtualMouse::~EvdevVirtualMouse()
{
    reset();
}

QString EvdevVirtualMouse::path() const
{
    return m_device ? m_device->devNode() : QString();
}

void EvdevVirtualMouse::mouseButton(uint32_t button, bool state)
{
    if (!m_device) {
        return;
    }

    m_device->writeEvent(EV_KEY, button, state);
    m_device->writeSynReportEvent();
    VirtualMouse::mouseButton(button, state);
}

void EvdevVirtualMouse::mouseMotion(const QPointF &pos)
{
    if (!m_device) {
        return;
    }

    m_motionDelta += pos;
    auto syn = false;
    if (std::abs(m_motionDelta.x()) > 1) {
        m_device->writeEvent(EV_REL, REL_X, static_cast<int32_t>(m_motionDelta.x()));
        m_motionDelta.setX(std::fmod(m_motionDelta.x(), 1));
        syn = true;
    }
    if (std::abs(m_motionDelta.y()) > 1) {
        m_device->writeEvent(EV_REL, REL_Y, static_cast<int32_t>(m_motionDelta.y()));
        m_motionDelta.setY(std::fmod(m_motionDelta.y(), 1));
        syn = true;
    }
    if (syn) {
        m_device->writeSynReportEvent();
    }
}

void EvdevVirtualMouse::mouseWheel(const QPointF &delta)
{
    if (!m_device) {
        return;
    }

    m_wheelDelta += delta;
    auto syn = false;
    if (std::abs(m_wheelDelta.x()) > 1) {
        m_device->writeEvent(EV_REL, REL_HWHEEL_HI_RES, static_cast<int32_t>(m_wheelDelta.x()));
        m_wheelDelta.setX(std::fmod(m_wheelDelta.x(), 1));
        syn = true;
    }
    if (std::abs(m_wheelDelta.y()) > 1) {
        m_device->writeEvent(EV_REL, REL_WHEEL_HI_RES, -static_cast<int32_t>(m_wheelDelta.y()));
        m_wheelDelta.setY(std::fmod(m_wheelDelta.y(), 1));
        syn = true;
    }
    if (syn) {
        m_device->writeSynReportEvent();
    }
}

}