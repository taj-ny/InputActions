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

#include <libinputactions/input/backends/InputBackend.h>

#include <hyprland/src/devices/IPointer.hpp>
#include <hyprland/src/plugins/HookSystem.hpp>
#include <hyprland/src/SharedDefs.hpp>
#undef HANDLE

class Plugin;

class HyprlandInputBackend : public libinputactions::InputBackend
{
public:
    HyprlandInputBackend(Plugin *plugin);
    ~HyprlandInputBackend() = default;

private:
    bool pinchBegin(const uint32_t &fingers);
    bool pinchUpdate(const double &scale, const double &angleDelta);
    bool pinchEnd(const bool &cancelled);

    bool swipeBegin(const uint32_t &fingers);
    bool swipeUpdate(const Vector2D &delta);
    bool swipeEnd(const bool &cancelled);

    std::vector<SP<HOOK_CALLBACK_FN>> m_events;

    libinputactions::InputDevice m_fakeTouchpad;

    uint32_t m_fingers{};
    bool m_block{};
    bool m_emittingBeginEvent{};

    std::unique_ptr<CFunctionHook> m_pinchBeginHook;
    std::unique_ptr<CFunctionHook> m_pinchUpdateHook;
    std::unique_ptr<CFunctionHook> m_pinchEndHook;

    friend void pinchBeginHook(void *thisPtr, uint32_t timeMs, uint32_t fingers);
    friend void pinchUpdateHook(void *thisPtr, uint32_t timeMs, const Vector2D &delta, double scale, double rotation);
    friend void pinchEndHook(void *thisPtr, uint32_t timeMs, bool cancelled);
};