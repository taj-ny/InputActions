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

#include "InputDevice.h"
#include <libinputactions/handlers/TouchpadTriggerHandler.h>
#include <libinputactions/handlers/TouchscreenTriggerHandler.h>
#include <libinputactions/input/events.h>

namespace InputActions
{

static const std::chrono::milliseconds TOUCHSCREEN_SIMULATED_TAP_TURATION{10L};

InputDevice::InputDevice(InputDeviceType type, QString name, QString sysName)
    : m_type(type)
    , m_name(std::move(name))
    , m_sysName(std::move(sysName))
{
    m_touchscreenTapTimer.setSingleShot(true);
    m_touchscreenTapTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_touchscreenTapTimer, &QTimer::timeout, this, &InputDevice::onTouchscreenTapTimerTimeout);
}

InputDevice::~InputDevice() = default;

void InputDevice::keyboardKey(uint32_t key, bool state)
{
    m_virtualState.setKeyState(key, state);
}

void InputDevice::touchscreenTap(const std::vector<QPointF> &points)
{
    if (m_touchscreenTapTimer.isActive() || !m_physicalState.validTouchPoints().empty()) {
        return;
    }

    touchscreenTapDown(points);
    m_touchscreenTapPoints = points;
    m_touchscreenTapTimer.start(TOUCHSCREEN_SIMULATED_TAP_TURATION);
}

void InputDevice::onTouchscreenTapTimerTimeout()
{
    touchscreenTapUp(m_touchscreenTapPoints);
}

bool InputDevice::keyboardKey(const KeyboardKeyEvent &event)
{
    m_physicalState.setKeyState(event.nativeKey(), event.state());
    return false;
}

void InputDevice::handleNotBlockedEvent(const InputEvent &event)
{
    switch (event.type()) {
        case InputEventType::KeyboardKey:
            const auto &keyboardKeyEvent = dynamic_cast<const KeyboardKeyEvent &>(event);
            m_virtualState.setKeyState(keyboardKeyEvent.nativeKey(), keyboardKeyEvent.state());
            break;
    }
}

bool InputDevice::touchCancel(const TouchCancelEvent &event)
{
    for (auto &point : m_physicalState.touchPoints()) {
        point.active = false;
        point.valid = false;
    }

    return false;
}

bool InputDevice::touchDown(const TouchDownEvent &event)
{
    auto &points = m_physicalState.touchPoints();
    auto point = std::ranges::find_if(points, [](auto &point) {
        return !point.active;
    });
    if (point == points.end()) {
        points.push_back({});
        point = points.end() - 1;
    }

    point->active = true;
    point->valid = true;
    point->type = TouchPointType::Finger;
    point->initialPosition = event.position();
    point->position = event.position();
    point->id = event.id();
    point->downTimestamp = std::chrono::steady_clock::now();
    point->rawPosition = event.rawPosition();
    point->rawInitialPosition = event.rawPosition();
    point->pressure = event.pressure();
    updatePointState(event.id(), event.pressure());

    return false;
}

bool InputDevice::touchMotion(const TouchMotionEvent &event)
{
    auto *point = m_physicalState.findTouchPoint(event.id());
    if (!point) {
        return false;
    }

    point->position = event.position();
    point->rawPosition = event.rawPosition();

    return false;
}

bool InputDevice::touchPressureChange(const TouchPressureChangeEvent &event)
{
    auto *point = m_physicalState.findTouchPoint(event.id());
    if (!point) {
        return false;
    }

    point->pressure = event.pressure();
    updatePointState(event.id(), event.pressure());

    return false;
}

bool InputDevice::touchUp(const TouchUpEvent &event)
{
    auto *point = m_physicalState.findTouchPoint(event.id());
    if (!point) {
        return false;
    }

    point->active = false;
    point->valid = false;

    return false;
}

void InputDevice::updatePointState(int32_t id, uint32_t pressure)
{
    auto *point = m_physicalState.findTouchPoint(id);
    if (pressure >= m_properties.palmPressure()) {
        point->type = TouchPointType::Palm;
    } else if (pressure >= m_properties.thumbPressure()) {
        point->type = TouchPointType::Thumb;
    } else if (pressure >= m_properties.fingerPressure()) {
        point->type = TouchPointType::Finger;
    } else {
        point->type = TouchPointType::None;
    }
    point->valid = point->type == TouchPointType::Finger || point->type == TouchPointType::Thumb;
}

void InputDevice::setTouchpadTriggerHandler(std::unique_ptr<TouchpadTriggerHandler> value)
{
    m_touchpadTriggerHandler = std::move(value);
}

void InputDevice::setTouchscreenTriggerHandler(std::unique_ptr<TouchscreenTriggerHandler> value)
{
    m_touchscreenTriggerHandler = std::move(value);
}

}