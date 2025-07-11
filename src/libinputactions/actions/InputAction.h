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

#include "Action.h"
#include <QPointF>
#include <libinputactions/Value.h>

namespace libinputactions
{

class InputAction : public Action
{
public:
    /**
     * Input actions are performed in order as defined in the struct.
     */
    struct Item
    {
        std::vector<uint32_t> keyboardPress;
        std::vector<uint32_t> keyboardRelease;
        Value<QString> keyboardText;

        std::vector<uint32_t> mousePress;
        std::vector<uint32_t> mouseRelease;

        QPointF mouseMoveAbsolute;
        QPointF mouseMoveRelative;
        bool mouseMoveRelativeByDelta{};
    };

    InputAction(std::vector<Item> sequence);

    void execute() const override;
    bool async() const override;

    /**
     * Temporary hack, do not set outside of TriggerAction.
     */
    QPointF m_deltaMultiplied;

private:
    std::vector<Item> m_sequence;
};

}