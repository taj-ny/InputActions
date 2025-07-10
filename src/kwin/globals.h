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

#include "core/inputdevice.h"
#include "input.h"
#include "kwin/input.h"

#ifdef KWIN_6_3_OR_GREATER
typedef KWin::KeyboardKeyState KeyboardKeyState;
#define KeyboardKeyStatePressed KWin::KeyboardKeyState::Pressed
#define KeyboardKeyStateReleased KWin::KeyboardKeyState::Released
#define PointerButtonStatePressed KWin::PointerButtonState::Pressed
#define PointerButtonStateReleased KWin::PointerButtonState::Released
#else
typedef KWin::InputRedirection::KeyboardKeyState KeyboardKeyState;
#define KeyboardKeyStatePressed KWin::InputRedirection::KeyboardKeyPressed
#define KeyboardKeyStateReleased KWin::InputRedirection::KeyboardKeyReleased
#define PointerButtonStatePressed KWin::InputRedirection::PointerButtonPressed
#define PointerButtonStateReleased KWin::InputRedirection::PointerButtonReleased
#endif