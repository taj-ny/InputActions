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

#include "HyprlandInputBackend.h"

#include <hyprland/src/plugins/PluginAPI.hpp>
#undef HANDLE

using namespace libinputactions;

HyprlandInputBackend::HyprlandInputBackend(void *pluginHandle)
    : m_fakeTouchpad(InputDeviceType::Touchpad)
{
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(pluginHandle, "swipeBegin", [this](void *, SCallbackInfo &info, std::any event) {
        info.cancelled = swipeBegin(std::any_cast<IPointer::SSwipeBeginEvent>(event).fingers);
    }));
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(pluginHandle, "swipeUpdate", [this](void *, SCallbackInfo &info, std::any event) {
        info.cancelled = swipeUpdate(std::any_cast<IPointer::SSwipeUpdateEvent>(event).delta);
    }));
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(pluginHandle, "swipeEnd", [this](void *, SCallbackInfo &info, std::any event) {
        info.cancelled = swipeEnd(std::any_cast<IPointer::SSwipeEndEvent>(event).cancelled);
    }));
}

bool HyprlandInputBackend::swipeBegin(const uint32_t &fingers)
{
    const TouchpadGestureLifecyclePhaseEvent event(&m_fakeTouchpad, TouchpadGestureLifecyclePhase::Begin, TriggerType::StrokeSwipe, fingers);
    return handleEvent(&event);
}

bool HyprlandInputBackend::swipeUpdate(const Vector2D &delta)
{
    const MotionEvent event(&m_fakeTouchpad, InputEventType::TouchpadSwipe, QPointF(delta.x, delta.y));
    return handleEvent(&event);
}

bool HyprlandInputBackend::swipeEnd(const bool &cancelled)
{
    const TouchpadGestureLifecyclePhaseEvent event(&m_fakeTouchpad, cancelled ? TouchpadGestureLifecyclePhase::Cancel : TouchpadGestureLifecyclePhase::End,
        TriggerType::StrokeSwipe);
    handleEvent(&event);
    return false;
}

