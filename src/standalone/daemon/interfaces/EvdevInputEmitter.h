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

#include <libinputactions/interfaces/InputEmitter.h>

namespace InputActions
{

class LibevdevUinputDevice;

/**
 * Keyboard keys must be registered before initialization.
 */
class EvdevInputEmitter : public InputEmitter
{
public:
    EvdevInputEmitter();
    ~EvdevInputEmitter() override;

    void initialize() override;
    void reset() override;

    void keyboardClearModifiers() override;
    void keyboardKey(uint32_t key, bool state, const InputDevice *target = nullptr) override;

    void mouseAxis(const QPointF &delta) override;
    void mouseButton(uint32_t button, bool state, const InputDevice *target = nullptr) override;
    void mouseMoveRelative(const QPointF &pos) override;

    /**
     * @return Path of the virtual keyboard, or an empty string on failure.
     */
    QString keyboardPath() const;
    /**
     * @return Path of the virtual mouse, or an empty string on failure.
     */
    QString mousePath() const;

private:
    std::unique_ptr<LibevdevUinputDevice> m_keyboard;
    std::unique_ptr<LibevdevUinputDevice> m_mouse;

    QPointF m_mouseAxisDelta;
    QPointF m_mouseMotionDelta;
};

}