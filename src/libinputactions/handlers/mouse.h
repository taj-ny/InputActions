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

#include <libinputactions/handlers/motion.h>

Q_DECLARE_LOGGING_CATEGORY(LIBINPUTACTIONS_HANDLER_MOUSE)

namespace libinputactions
{

/**
 * Handles mouse triggers: press, stroke, swipe, wheel.
 *
 * Press triggers activate after a small delay in order to allow for normal clicks and dragging. This behavior can be
 * changed by making a press trigger instant, however any activated instant trigger will make all other activated
 * triggers instant as well.
 * @see setMotionTimeout
 * @see PressTrigger::setInstant
 *
 * Wheel triggers have a different lifecycle than other triggers, as they are activated, updated and ended on
 * every single event. They are never cancelled and do not support speed or thresholds, although there are no checks
 * against that.
 */
class MouseTriggerHandler : public MotionTriggerHandler
{
public:
    MouseTriggerHandler();

    bool handleEvent(const InputEvent *event) override;

    /**
     * The amount of time in milliseconds the handler will wait for motion to be performed (wheel is considered motion
     * as well) before attempting to activate press triggers. For pointer motion there is a small threshold to prevent
     * accidental activations.
     */
    void setMotionTimeout(const uint32_t &timeout);
    /**
     * The amount of time in milliseconds the handler will wait for all mouse buttons to be pressed before activating
     * press triggers.
     */
    void setPressTimeout(const uint32_t &timeout);

    /**
     * @param unblock Whether blocked mouse buttons should be pressed immediately on timeout. If false, they will be
     * pressed and instantly released on button release.
     */
    void setUnblockButtonsOnTimeout(const bool &unblock);

protected:
    void triggerActivating(const Trigger *trigger) override;

    /**
     * This implementation sets mouse buttons.
     * @see TriggerHandler::createActivationEvent
     */
    std::unique_ptr<TriggerActivationEvent> createActivationEvent() const override;

private:
    bool handleEvent(const MouseButtonEvent *event);
    bool handleMotionEvent(const MotionEvent *event);
    bool handleWheelEvent(const MotionEvent *event);

    /**
     * Checks whether there is an activatable trigger that uses the specified button. Mouse buttons are ignored when
     * checking activatibility. If a trigger has multiple buttons, all of them will be blocked, even if only one was
     * pressed.
     */
    bool shouldBlockMouseButton(const Qt::MouseButton &button);
    /**
     * Presses all currently blocked mouse buttons without releasing them.
     */
    void pressBlockedMouseButtons();

    /**
     * Used to wait until all mouse buttons have been pressed to avoid conflicts with gestures that require more than
     * one button.
     */
    QTimer m_pressTimeoutTimer;
    uint32_t m_pressTimeout = 50;
    QTimer m_motionTimeoutTimer;
    uint32_t m_motionTimeout = 200;

    /**
     * Activation event for the last button press.
     */
    std::unique_ptr<TriggerActivationEvent> m_activationEvent;

    bool m_instantPress = false;
    qreal m_mouseMotionSinceButtonPress = 0;
    bool m_hadMouseGestureSinceButtonPress = false;

    QList<quint32> m_blockedMouseButtons;
    Qt::MouseButtons m_buttons{};
    bool m_unblockButtonsOnTimeout = true;
};

}