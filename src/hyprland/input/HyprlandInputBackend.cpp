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
#include "Plugin.h"

#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/managers/SeatManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/protocols/PointerGestures.hpp>
#undef HANDLE

using namespace libinputactions;

typedef void (*holdBegin)(void *thisPtr, uint32_t timeMs, uint32_t fingers);
typedef void (*holdEnd)(void *thisPtr, uint32_t timeMs, bool cancelled);
typedef void (*pinchBegin)(void *thisPtr, uint32_t timeMs, uint32_t fingers);
typedef void (*pinchUpdate)(void *thisPtr, uint32_t timeMs, const Vector2D &delta, double scale, double rotation);
typedef void (*pinchEnd)(void *thisPtr, uint32_t timeMs, bool cancelled);

void holdBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers)
{
    auto *instance = static_cast<HyprlandInputBackend *>(InputBackend::instance());
    if (!instance->holdBegin(fingers)) {
        (*(holdBegin)instance->m_holdBeginHook->m_original)(thisPtr, timeMs, fingers);
    }
}

void holdEndHook(void *thisPtr, uint32_t timeMs, bool cancelled)
{
    auto *instance = static_cast<HyprlandInputBackend *>(InputBackend::instance());
    if (!instance->holdEnd(cancelled)) {
        (*(holdEnd)instance->m_holdEndHook->m_original)(thisPtr, timeMs, cancelled);
    }
}

void pinchBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers)
{
    auto *instance = static_cast<HyprlandInputBackend *>(InputBackend::instance());
    if (!instance->pinchBegin(fingers)) {
        (*(pinchBegin)instance->m_pinchBeginHook->m_original)(thisPtr, timeMs, fingers);
    }
}

void pinchUpdateHook(void *thisPtr, uint32_t timeMs, const Vector2D &delta, double scale, double rotation)
{
    auto *instance = static_cast<HyprlandInputBackend *>(InputBackend::instance());
    if (!instance->pinchUpdate(scale, rotation)) {
        (*(pinchUpdate)instance->m_pinchUpdateHook->m_original)(thisPtr, timeMs, delta, scale, rotation);
    }
}

void pinchEndHook(void *thisPtr, uint32_t timeMs, bool cancelled)
{
    auto *instance = static_cast<HyprlandInputBackend *>(InputBackend::instance());
    if (!instance->pinchEnd(cancelled)) {
        (*(pinchEnd)instance->m_pinchEndHook->m_original)(thisPtr, timeMs, cancelled);
    }
}

HyprlandInputBackend::HyprlandInputBackend(Plugin *plugin)
    : m_fakeTouchpad(InputDeviceType::Touchpad)
{
    auto handle = plugin->handle();
    m_holdBeginHook.reset(HyprlandAPI::createFunctionHook(handle, HyprlandAPI::findFunctionsByName(handle, "holdBegin")[0].address, (void *) &holdBeginHook));
    m_holdBeginHook->hook();
    m_holdEndHook.reset(HyprlandAPI::createFunctionHook(handle, HyprlandAPI::findFunctionsByName(handle, "holdEnd")[0].address, (void *) &holdEndHook));
    m_holdEndHook->hook();
    m_pinchBeginHook.reset(HyprlandAPI::createFunctionHook(handle, HyprlandAPI::findFunctionsByName(handle, "pinchBegin")[0].address, (void *) &pinchBeginHook));
    m_pinchBeginHook->hook();
    m_pinchUpdateHook.reset(HyprlandAPI::createFunctionHook(handle, HyprlandAPI::findFunctionsByName(handle, "pinchUpdate")[0].address, (void *) &pinchUpdateHook));
    m_pinchUpdateHook->hook();
    m_pinchEndHook.reset(HyprlandAPI::createFunctionHook(handle, HyprlandAPI::findFunctionsByName(handle, "pinchEnd")[0].address, (void *)&pinchEndHook));
    m_pinchEndHook->hook();

    m_events.push_back(HyprlandAPI::registerCallbackDynamic(handle, "swipeBegin", [this](void *, SCallbackInfo &info, std::any event) {
        info.cancelled = swipeBegin(std::any_cast<IPointer::SSwipeBeginEvent>(event).fingers);
    }));
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(handle, "swipeUpdate", [this](void *, SCallbackInfo &info, std::any event) {
        info.cancelled = swipeUpdate(std::any_cast<IPointer::SSwipeUpdateEvent>(event).delta);
    }));
    m_events.push_back(HyprlandAPI::registerCallbackDynamic(handle, "swipeEnd", [this](void *, SCallbackInfo &info, std::any event) {
        info.cancelled = swipeEnd(std::any_cast<IPointer::SSwipeEndEvent>(event).cancelled);
    }));
}

bool HyprlandInputBackend::holdBegin(const uint32_t &fingers)
{
    m_fingers = fingers;
    const TouchpadGestureLifecyclePhaseEvent event(&m_fakeTouchpad, TouchpadGestureLifecyclePhase::Begin, TriggerType::Press, fingers);
    m_block = handleEvent(&event);
    return m_block;
}

bool HyprlandInputBackend::holdEnd(const bool &cancelled)
{
    const TouchpadGestureLifecyclePhaseEvent event(&m_fakeTouchpad, cancelled ? TouchpadGestureLifecyclePhase::Cancel : TouchpadGestureLifecyclePhase::End,
        TriggerType::Press);
    handleEvent(&event);
    return m_block;
}

bool HyprlandInputBackend::pinchBegin(const uint32_t &fingers)
{
    if (m_emittingBeginEvent) {
        return false;
    }

    m_fingers = fingers;
    const TouchpadGestureLifecyclePhaseEvent event(&m_fakeTouchpad, TouchpadGestureLifecyclePhase::Begin, TriggerType::PinchRotate, fingers);
    m_block = handleEvent(&event);
    return m_block;
}

bool HyprlandInputBackend::pinchUpdate(const double &scale, const double &angleDelta)
{
    const TouchpadPinchEvent event(&m_fakeTouchpad, scale, angleDelta);
    const auto block = handleEvent(&event);
    if (m_block && !block) {
        m_emittingBeginEvent = true;
        PROTO::pointerGestures->pinchBegin(0, m_fingers);
        m_emittingBeginEvent = false;
    }
    m_block = block;
    return block;
}

bool HyprlandInputBackend::pinchEnd(const bool &cancelled)
{
    const TouchpadGestureLifecyclePhaseEvent event(&m_fakeTouchpad, cancelled ? TouchpadGestureLifecyclePhase::Cancel : TouchpadGestureLifecyclePhase::End,
        TriggerType::PinchRotate);
    return handleEvent(&event);
}

bool HyprlandInputBackend::swipeBegin(const uint32_t &fingers)
{
    if (m_emittingBeginEvent) {
        return false;
    }

    m_fingers = fingers;
    const TouchpadGestureLifecyclePhaseEvent event(&m_fakeTouchpad, TouchpadGestureLifecyclePhase::Begin, TriggerType::StrokeSwipe, fingers);
    m_block = handleEvent(&event);
    return m_block;
}

bool HyprlandInputBackend::swipeUpdate(const Vector2D &delta)
{
    const MotionEvent event(&m_fakeTouchpad, InputEventType::TouchpadSwipe, QPointF(delta.x, delta.y));
    const auto block = handleEvent(&event);
    if (m_block && !block) {
        m_emittingBeginEvent = true;
        g_pInputManager->onSwipeBegin(IPointer::SSwipeBeginEvent{
            .fingers = m_fingers
        });
        m_emittingBeginEvent = false;
    }
    m_block = block;
    return block;
}

bool HyprlandInputBackend::swipeEnd(const bool &cancelled)
{
    const TouchpadGestureLifecyclePhaseEvent event(&m_fakeTouchpad, cancelled ? TouchpadGestureLifecyclePhase::Cancel : TouchpadGestureLifecyclePhase::End,
        TriggerType::StrokeSwipe);
    return handleEvent(&event);
}

