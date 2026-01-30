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

#include "WheelTrigger.h"

namespace InputActions
{

WheelTrigger::WheelTrigger(TriggerDirection direction)
    : DirectionalMotionTrigger(TriggerType::Wheel, direction)
{
}

void WheelTrigger::actionAdded(TriggerAction *action)
{
    DirectionalMotionTrigger::actionAdded(action);
    if (action->on() == On::Update) {
        m_continuous = true;
    }
}

bool WheelTrigger::continuous() const
{
    return m_continuous;
}

}