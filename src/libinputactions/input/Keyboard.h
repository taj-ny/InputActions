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

#include <map>
#include <memory>

#include <QPointF>

#include <linux/input-event-codes.h>

namespace libinputactions
{

static std::map<uint32_t, Qt::KeyboardModifier> MODIFIERS = {
    {KEY_LEFTALT, Qt::KeyboardModifier::AltModifier},
    {KEY_LEFTCTRL, Qt::KeyboardModifier::ControlModifier},
    {KEY_LEFTMETA, Qt::KeyboardModifier::MetaModifier},
    {KEY_LEFTSHIFT, Qt::KeyboardModifier::ShiftModifier},
    {KEY_RIGHTALT, Qt::KeyboardModifier::AltModifier},
    {KEY_RIGHTCTRL, Qt::KeyboardModifier::ControlModifier},
    {KEY_RIGHTMETA, Qt::KeyboardModifier::MetaModifier},
    {KEY_RIGHTSHIFT, Qt::KeyboardModifier::ShiftModifier},
};

class KeyboardKeyEvent;

class Keyboard
{
    INPUTACTIONS_DECLARE_SINGLETON(Keyboard)

public:
    virtual ~Keyboard() = default;

    void handleEvent(const KeyboardKeyEvent *event);

    /**
     * @return Currently pressed keyboard modifiers
     * @remark Key events that have been ignored by the input backend will not be used to update the modifier state. For example, clearing modifiers will not
     * update the modifier state to none. This allows gestures with keyboard modifier conditions to be used again.
     */
    const Qt::KeyboardModifiers &modifiers() const;

protected:
    Keyboard() = default;

private:
    Qt::KeyboardModifiers m_modifiers{};
};

}