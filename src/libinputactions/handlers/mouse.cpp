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

#include "mouse.h"

#include <libinputactions/input/emitter.h>
#include <libinputactions/input/keyboard.h>
#include <libinputactions/input/pointer.h>
#include <libinputactions/triggers/press.h>
#include <libinputactions/triggers/wheel.h>

Q_LOGGING_CATEGORY(LIBINPUTACTIONS_HANDLER_MOUSE, "libinputactions.handler.mouse", QtWarningMsg)

namespace libinputactions
{

MouseTriggerHandler::MouseTriggerHandler()
    : MotionTriggerHandler()
{
    m_pressTimeoutTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_pressTimeoutTimer.setSingleShot(true);

    m_motionTimeoutTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_motionTimeoutTimer.setSingleShot(true);
}

bool MouseTriggerHandler::handleEvent(const InputEvent *event)
{
    MotionTriggerHandler::handleEvent(event);
    switch (event->type()) {
        case InputEventType::MouseButton:
            return handleEvent(static_cast<const MouseButtonEvent *>(event));
        case InputEventType::MouseMotion:
            return handleMotionEvent(static_cast<const MotionEvent *>(event));
        case InputEventType::MouseWheel:
            return handleWheelEvent(static_cast<const MotionEvent *>(event));
        default:
            return false;
    }
}

bool MouseTriggerHandler::handleEvent(const MouseButtonEvent *event)
{
    const auto &button = event->button();
    const auto &nativeButton = event->nativeButton();
    const auto &state = event->state();
    qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE).nospace() << "Event (type: PointerMotion, button: " << button << ", state: " << state << ")";

    endTriggers(TriggerType::Wheel);
    if (state) {
        m_mouseMotionSinceButtonPress = 0;
        m_hadMouseGestureSinceButtonPress = false;
        m_buttons |= button;

        cancelTriggers(TriggerType::All);
        m_activationEvent = createActivationEvent();

        // This should be per-gesture instead of global, but it's good enough
        m_instantPress = false;
        for (const auto &trigger : triggers(TriggerType::Press, m_activationEvent.get())) {
            if (dynamic_cast<PressTrigger *>(trigger)->instant()) {
                qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "Press gesture is instant");
                m_instantPress = true;
                break;
            }
        }

        m_motionTimeoutTimer.stop();
        disconnect(&m_pressTimeoutTimer, nullptr, nullptr, nullptr);
        disconnect(&m_motionTimeoutTimer, nullptr, nullptr, nullptr);
        connect(&m_pressTimeoutTimer, &QTimer::timeout, this, [this] {
            const auto swipeTimeout = [this] {
                if (m_hadMouseGestureSinceButtonPress) {
                    qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "Mouse gesture updated before motion timeout");
                    return;
                }

                qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "Attempting to activate mouse press gestures");
                if (!activateTriggers(TriggerType::Press, m_activationEvent.get())) {
                    qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "No wheel or press mouse gestures");
                    if (m_unblockButtonsOnTimeout) {
                        pressBlockedMouseButtons();
                    }
                }
            };

            if (m_instantPress) {
                swipeTimeout();
                return;
            }

            connect(&m_motionTimeoutTimer, &QTimer::timeout, [swipeTimeout] {
                qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "No mouse motion");
                swipeTimeout();
            });
            m_motionTimeoutTimer.start(m_motionTimeout);
            qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "Waiting for mouse motion");
        });
        m_pressTimeoutTimer.start(m_pressTimeout);
        qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "Waiting for all mouse buttons");

        if (shouldBlockMouseButton(button)) {
            m_blockedMouseButtons.push_back(nativeButton);
            return true;
        }
    } else {
        m_buttons &= ~button;
        endTriggers(TriggerType::All);

        // Prevent gesture skipping when clicking rapidly
        if (m_pressTimeoutTimer.isActive() || m_motionTimeoutTimer.isActive()) {
            m_pressTimeoutTimer.stop();
            m_motionTimeoutTimer.stop();

            if (m_instantPress) {
                activateTriggers(TriggerType::Press, m_activationEvent.get());
                pressUpdate();
                endTriggers(TriggerType::Press);
            }
        }

        const auto block = m_blockedMouseButtons.contains(nativeButton);
        if (m_blockedMouseButtons.removeAll(nativeButton) && !m_hadMouseGestureSinceButtonPress) {
            qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE).nospace() << "Mouse button pressed and released (button: " << nativeButton << ")";
            InputEmitter::instance()->mouseButton(nativeButton, true);
            InputEmitter::instance()->mouseButton(nativeButton, false);
        }
        if (m_blockedMouseButtons.empty()) {
            m_hadMouseGestureSinceButtonPress = false;
        }
        return block;
    }

    return false;
}

bool MouseTriggerHandler::handleMotionEvent(const MotionEvent *event)
{
    const auto &delta = event->delta();
    qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE).nospace() << "Event (type: PointerMotion, delta: " << delta << ")";

    if (m_pressTimeoutTimer.isActive()) {
        qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "Event processed (type: PointerMotion, status: PressingButtons)");
        return true;
    }

    m_mouseMotionSinceButtonPress += std::hypot(delta.x(), delta.y());
    if (m_mouseMotionSinceButtonPress < 5) {
        qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE).nospace() << "Event processed (type: PointerMotion, status: InsufficientMotion, delta: " << delta << ")";
        return true;
    }

    if (!hasActiveTriggers(TriggerType::All & ~TriggerType::Press)) {
        cancelTriggers(TriggerType::All);
        m_motionTimeoutTimer.stop();

        qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "Attempting to activate mouse motion gestures");
        if (!activateTriggers(TriggerType::StrokeSwipe)) {
            qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "No motion gestures");
            pressBlockedMouseButtons();
        }
    }

    const auto hadActiveGestures = hasActiveTriggers(TriggerType::StrokeSwipe);
    const auto block = handleMotion(delta);
    if (hadActiveGestures && !hasActiveTriggers(TriggerType::StrokeSwipe)) {
        qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "Mouse motion gesture ended/cancelled during motion");
        // Swipe gesture cancelled due to wrong speed or direction
        pressBlockedMouseButtons();
    }
    return block;
}

bool MouseTriggerHandler::handleWheelEvent(const MotionEvent *event)
{
    const auto &delta = event->delta();
    qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE).nospace() << "Event (type: Wheel, delta: " << delta << ")";

    if (!hasActiveTriggers(TriggerType::Wheel) && !activateTriggers(TriggerType::Wheel)) {
        qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "Event processed (type: Wheel, status: NoGestures)");
        return false;
    }

    SwipeDirection direction = SwipeDirection::Left;
    if (delta.x() > 0) {
        direction = SwipeDirection::Right;
    } else if (delta.y() > 0) {
        direction = SwipeDirection::Down;
    } else if (delta.y() < 0) {
        direction = SwipeDirection::Up;
    }
    DirectionalMotionTriggerUpdateEvent updateEvent;
    updateEvent.setDelta(delta.x() != 0 ? delta.x() : delta.y());
    updateEvent.setDirection(static_cast<TriggerDirection>(direction));

    const auto hasTriggers = updateTriggers(TriggerType::Wheel, &updateEvent);
    bool continuous = false;
    for (const auto &trigger : activeTriggers(TriggerType::Wheel)) {
        if (static_cast<WheelTrigger *>(trigger)->continuous()) {
            continuous = true;
        }
    }
    if (!continuous || (!m_buttons && !Keyboard::instance()->modifiers())) {
        qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE, "Wheel trigger will end immediately");
        endTriggers(TriggerType::Wheel);
    }

    qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE).noquote().nospace() << "Event processed (type: Wheel, hasGestures: " << hasTriggers << ")";
    return hasTriggers;
}

void MouseTriggerHandler::setMotionTimeout(const uint32_t &timeout)
{
    m_motionTimeout = timeout;
}

void MouseTriggerHandler::setPressTimeout(const uint32_t &timeout)
{
    m_pressTimeout = timeout;
}

void MouseTriggerHandler::triggerActivating(const Trigger *trigger)
{
    MotionTriggerHandler::triggerActivating(trigger);
    m_hadMouseGestureSinceButtonPress = true;
}

std::unique_ptr<TriggerActivationEvent> MouseTriggerHandler::createActivationEvent() const
{
    auto event = TriggerHandler::createActivationEvent();
    event->mouseButtons = m_buttons;
    return event;
}

bool MouseTriggerHandler::shouldBlockMouseButton(const Qt::MouseButton &button)
{
    const auto event = createActivationEvent();
    // A partial match is required, not an exact one
    event->mouseButtons = std::nullopt;
    for (const auto &trigger : triggers(TriggerType::All, event.get())) {
        const auto buttons = trigger->mouseButtons();
        if (buttons && (*buttons & button)) {
            qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE).noquote().nospace() << "Mouse button blocked (button: " << button << ", trigger: " << trigger->name() << ")";
            return true;
        }
    }
    return false;
}

void MouseTriggerHandler::pressBlockedMouseButtons()
{
    for (const auto &button : m_blockedMouseButtons) {
        InputEmitter::instance()->mouseButton(button, true);
        qCDebug(LIBINPUTACTIONS_HANDLER_MOUSE).nospace() << "Mouse button unblocked (button: " << button << ")";
    }
    m_blockedMouseButtons.clear();
}

void MouseTriggerHandler::setUnblockButtonsOnTimeout(const bool &unblock)
{
    m_unblockButtonsOnTimeout = unblock;
}

}