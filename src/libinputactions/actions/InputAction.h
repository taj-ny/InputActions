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
#include <chrono>
#include <libinputactions/Value.h>

namespace InputActions
{

class InputAction : public Action
{
public:
    /**
     * Only one member may be set.
     */
    struct Item
    {
        uint32_t keyboardPress{};
        uint32_t keyboardRelease{};
        Value<QString> keyboardText;

        uint32_t mousePress{};
        uint32_t mouseRelease{};

        QPointF mouseAxis;
        QPointF mouseMoveAbsolute;
        QPointF mouseMoveRelative;
        bool mouseMoveRelativeByDelta{};
    };

    InputAction(std::vector<Item> sequence);

    bool async() const override;
    bool mergeable() const override;

    /**
     * Delay between each item in the sequence.
     */
    const std::chrono::milliseconds &delay() const { return m_delay; }
    void setDelay(std::chrono::milliseconds value) { m_delay = std::move(value); }

    /**
     * Temporary hack, do not set outside of TriggerAction.
     */
    QPointF m_deltaMultiplied;

protected:
    void executeImpl(uint32_t executions) override;

private:
    std::vector<Item> m_sequence;
    std::chrono::milliseconds m_delay{};
};

}