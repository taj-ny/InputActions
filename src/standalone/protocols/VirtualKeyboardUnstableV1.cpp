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

#include "VirtualKeyboardUnstableV1.h"
#include "WlSeat.h"
#include <libinputactions/globals.h>
#include <sys/mman.h>

VirtualKeyboardUnstableV1::VirtualKeyboardUnstableV1()
    : WaylandProtocol(zwp_virtual_keyboard_manager_v1_interface.name)
{
}

VirtualKeyboardUnstableV1::~VirtualKeyboardUnstableV1()
{
    if (m_manager) {
        zwp_virtual_keyboard_manager_v1_destroy(m_manager);
    }
}

std::unique_ptr<VirtualKeyboardUnstableV1Keyboard> VirtualKeyboardUnstableV1::createKeyboard()
{
    auto keyboard = std::make_unique<VirtualKeyboardUnstableV1Keyboard>(m_manager);
    if (!keyboard->valid()) {
        return {};
    }
    return keyboard;
}

void VirtualKeyboardUnstableV1::bind(wl_registry *registry, uint32_t name, uint32_t version)
{
    WaylandProtocol::bind(registry, name, version);
    m_manager = static_cast<zwp_virtual_keyboard_manager_v1 *>(wl_registry_bind(registry, name, &zwp_virtual_keyboard_manager_v1_interface, version));
}

VirtualKeyboardUnstableV1Keyboard::VirtualKeyboardUnstableV1Keyboard(zwp_virtual_keyboard_manager_v1 *manager)
{
    m_keyboard = zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(manager, g_wlSeat->seat());

    xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (!context) {
        qCDebug(INPUTACTIONS, "Failed to create xkb context");
        return;
    }

    // TODO allow keymap configuration
    m_keymap = xkb_keymap_new_from_names(context, nullptr, XKB_KEYMAP_COMPILE_NO_FLAGS);
    if (!m_keymap) {
        qCDebug(INPUTACTIONS, "Failed to create xkb keymap");
        return;
    }
    char *keymap = xkb_keymap_get_as_string(m_keymap, XKB_KEYMAP_FORMAT_TEXT_V1);

    int fd = memfd_create("keymap", 0);
    if (fd == -1) {
        qCDebug(INPUTACTIONS) << "Failed to create keymap memfd (errno: " << errno << ")";
        return;
    }
    const auto length = strlen(keymap);
    if (write(fd, keymap, length) != length) {
        qCDebug(INPUTACTIONS) << "Failed to write keymap to memfd (errno: " << errno << ")";
        return;
    }
    lseek(fd, 0, SEEK_SET);

    zwp_virtual_keyboard_v1_keymap(m_keyboard, WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, fd, length);
    xkb_context_unref(context);
    free(keymap);
    close(fd);
    m_valid = true;
}

VirtualKeyboardUnstableV1Keyboard::~VirtualKeyboardUnstableV1Keyboard()
{
    if (m_keyboard) {
        zwp_virtual_keyboard_v1_destroy(m_keyboard);
    }
    if (m_keymap) {
        xkb_keymap_unref(m_keymap);
    }
}

void VirtualKeyboardUnstableV1Keyboard::key(uint32_t key, bool state)
{
    zwp_virtual_keyboard_v1_key(m_keyboard, 0, key, state ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED);
}

void VirtualKeyboardUnstableV1Keyboard::modifiers(Qt::KeyboardModifiers modifiers)
{
    static const std::unordered_map<Qt::KeyboardModifier, QString> xkbModifiers{{Qt::KeyboardModifier::AltModifier, "Alt"},
                                                                                {Qt::KeyboardModifier::ControlModifier, "Control"},
                                                                                {Qt::KeyboardModifier::MetaModifier, "Super"},
                                                                                {Qt::KeyboardModifier::ShiftModifier, "Shift"}};
    uint32_t mask{};
    for (const auto &[modifier, name] : xkbModifiers) {
        if (modifiers & modifier) {
            mask |= xkb_keymap_mod_get_mask(m_keymap, name.toStdString().c_str());
        }
    }
    zwp_virtual_keyboard_v1_modifiers(m_keyboard, mask, 0, 0, 0);
}

const bool &VirtualKeyboardUnstableV1Keyboard::valid() const
{
    return m_valid;
}