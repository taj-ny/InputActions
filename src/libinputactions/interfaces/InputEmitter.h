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

#include <libinputactions/globals.h>

#include <memory>

#include <QPointF>

namespace libinputactions
{

class InputEmitter
{
    INPUTACTIONS_DECLARE_SINGLETON(InputEmitter)

public:
    virtual ~InputEmitter() = default;

    /**
     * @param key <linux/input-event-codes.h>
     * @param state True - press, false - release
     */
    virtual void keyboardKey(const uint32_t &key, const bool &state);

    /**
     * @param button <linux/input-event-codes.h>
     * @param state True - press, false - release
     */
    virtual void mouseButton(const uint32_t &button, const bool &state);
    virtual void mouseMoveRelative(const QPointF &pos);

    virtual void touchpadPinchBegin(const uint8_t &fingers);
    virtual void touchpadSwipeBegin(const uint8_t &fingers);

protected:
    InputEmitter() = default;
};

}