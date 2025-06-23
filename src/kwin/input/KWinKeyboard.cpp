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

#include "KWinKeyboard.h"

#include "workspace.h"

#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/Keyboard.h>
#include <libinputactions/interfaces/InputEmitter.h>

void KWinKeyboard::clearModifiers()
{
    // Prevent modifier-only global shortcuts from being triggered. Clients will still see the event and may perform
    // actions.
    const auto globalShortcutsDisabled = KWin::workspace()->globalShortcutsDisabled();
    if (!globalShortcutsDisabled) {
        KWin::workspace()->disableGlobalShortcutsForClient(true);
    }

    auto emitter = libinputactions::InputEmitter::instance();
    const auto modifiers = libinputactions::Keyboard::instance()->modifiers(); // This is not the real state, but it's fine in this case.
    for (const auto &[key, modifier] : libinputactions::s_modifiers) {
        if (modifiers & modifier) {
            emitter->keyboardKey(key, false);
        }
    }

    if (!globalShortcutsDisabled) {
        KWin::workspace()->disableGlobalShortcutsForClient(false);
    }
}
