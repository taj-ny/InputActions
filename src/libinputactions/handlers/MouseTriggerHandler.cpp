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

#include "MouseTriggerHandler.h"
#include <libinputactions/input/backends/InputBackend.h>
#include <libinputactions/input/devices/InputDevice.h>
#include <libinputactions/input/events.h>
#include <libinputactions/triggers/PressTrigger.h>
#include <libinputactions/triggers/WheelTrigger.h>
#include <ranges>

Q_LOGGING_CATEGORY(INPUTACTIONS_HANDLER_MOUSE, "inputactions.handler.mouse", QtWarningMsg)

namespace InputActions
{

MouseTriggerHandler::MouseTriggerHandler()
{
    setDeviceTypes(InputDeviceType::Mouse);

    connect(this, &TriggerHandler::activatingTrigger, this, &MouseTriggerHandler::onActivatingTrigger);

    m_pressTimeoutTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_pressTimeoutTimer.setSingleShot(true);

    m_motionTimeoutTimer.setTimerType(Qt::TimerType::PreciseTimer);
    m_motionTimeoutTimer.setSingleShot(true);
}

bool MouseTriggerHandler::keyboardKey(const KeyboardKeyEvent &event)
{
    MotionTriggerHandler::keyboardKey(event);

    // If a modifier is released before mouse button, this will mess up blocking
    if (m_blockedMouseButtons.empty()) {
        m_hadTriggerSincePress = false;
    }
    return false;
}

bool MouseTriggerHandler::pointerAxis(const MotionEvent &event)
{
    const auto &delta = event.delta().unaccelerated();
    qCDebug(INPUTACTIONS_HANDLER_MOUSE).nospace() << "Event (type: Wheel, delta: " << delta << ")";

    if (!hasActiveTriggers(TriggerType::Wheel) && !activateTriggers(TriggerType::Wheel).success) {
        qCDebug(INPUTACTIONS_HANDLER_MOUSE, "Event processed (type: Wheel, status: NoGestures)");
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

    const auto result = updateTriggers(TriggerType::Wheel, updateEvent);
    bool continuous = false;
    for (const auto &trigger : activeTriggers(TriggerType::Wheel)) {
        if (static_cast<WheelTrigger *>(trigger)->continuous()) {
            continuous = true;
        }
    }
    if (!continuous || (m_buttons.empty() && !g_inputBackend->keyboardModifiers())) {
        qCDebug(INPUTACTIONS_HANDLER_MOUSE, "Wheel trigger will end immediately");
        endTriggers(TriggerType::Wheel);
    }

    qCDebug(INPUTACTIONS_HANDLER_MOUSE).noquote().nospace() << "Event processed (type: Wheel, hasGestures: " << result.success << ")";
    return result.block;
}

bool MouseTriggerHandler::pointerButton(const PointerButtonEvent &event)
{
    const auto button = event.button();
    const auto &state = event.state();
    qCDebug(INPUTACTIONS_HANDLER_MOUSE).nospace() << "Event (type: PointerMotion, button: " << button.scanCode() << ", state: " << state << ")";

    endTriggers(TriggerType::Wheel);
    if (state) {
        m_mouseMotionSinceButtonPress = 0;
        m_hadTriggerSincePress = false;
        if (!std::ranges::contains(m_buttons, button)) {
            m_buttons.push_back(button);
        }

        cancelTriggers(TriggerType::All);
        m_activationEvent = createActivationEvent();

        // This should be per-gesture instead of global, but it's good enough
        m_instantPress = false;
        for (const auto &trigger : triggers(TriggerType::Press, *m_activationEvent)) {
            if (dynamic_cast<PressTrigger *>(trigger)->instant()) {
                qCDebug(INPUTACTIONS_HANDLER_MOUSE, "Press gesture is instant");
                m_instantPress = true;
                break;
            }
        }

        m_motionTimeoutTimer.stop();
        disconnect(&m_pressTimeoutTimer, nullptr, nullptr, nullptr);
        disconnect(&m_motionTimeoutTimer, nullptr, nullptr, nullptr);

        const auto motionTimeout = event.sender()->properties().mouseMotionTimeout();
        const auto unblockButtonsOnTimeout = event.sender()->properties().mouseUnblockButtonsOnTimeout();
        connect(&m_pressTimeoutTimer, &QTimer::timeout, this, [this, sender = event.sender(), motionTimeout, unblockButtonsOnTimeout] {
            const auto swipeTimeout = [this, sender, unblockButtonsOnTimeout] {
                if (m_hadTriggerSincePress) {
                    qCDebug(INPUTACTIONS_HANDLER_MOUSE, "Mouse gesture updated before motion timeout");
                    return;
                }

                qCDebug(INPUTACTIONS_HANDLER_MOUSE, "Attempting to activate mouse press gestures");
                if (!activateTriggers(TriggerType::Press, *m_activationEvent).success) {
                    qCDebug(INPUTACTIONS_HANDLER_MOUSE, "No wheel or press mouse gestures");
                    if (unblockButtonsOnTimeout) {
                        pressBlockedMouseButtons(sender);
                    }
                }
            };

            if (m_instantPress) {
                swipeTimeout();
                return;
            }

            connect(&m_motionTimeoutTimer, &QTimer::timeout, [swipeTimeout] {
                qCDebug(INPUTACTIONS_HANDLER_MOUSE, "No mouse motion");
                swipeTimeout();
            });
            m_motionTimeoutTimer.start(motionTimeout);
            qCDebug(INPUTACTIONS_HANDLER_MOUSE, "Waiting for mouse motion");
        });
        m_pressTimeoutTimer.start(event.sender()->properties().mousePressTimeout());
        qCDebug(INPUTACTIONS_HANDLER_MOUSE, "Waiting for all mouse buttons");

        if (shouldBlockMouseButton(button)) {
            m_blockedMouseButtons.push_back(button);
            return true;
        }
    } else {
        std::erase(m_buttons, button);
        endTriggers(TriggerType::All);

        // Prevent gesture skipping when clicking rapidly
        if (m_pressTimeoutTimer.isActive() || m_motionTimeoutTimer.isActive()) {
            m_pressTimeoutTimer.stop();
            m_motionTimeoutTimer.stop();

            if (m_instantPress) {
                activateTriggers(TriggerType::Press, *m_activationEvent);
                updateTimedTriggers();
                endTriggers(TriggerType::Press);
            }
        }

        const auto block = m_blockedMouseButtons.contains(button);
        if (m_blockedMouseButtons.removeAll(button) && !m_hadTriggerSincePress) {
            qCDebug(INPUTACTIONS_HANDLER_MOUSE).nospace() << "Mouse button pressed and released (button: " << button.scanCode() << ")";
            event.sender()->mouseButton(button, true);
            event.sender()->mouseButton(button, false);
        }
        if (m_blockedMouseButtons.empty()) {
            m_hadTriggerSincePress = false;
        }
        return block;
    }

    return false;
}

bool MouseTriggerHandler::pointerMotion(const MotionEvent &event)
{
    const auto &delta = event.delta();
    qCDebug(INPUTACTIONS_HANDLER_MOUSE).nospace() << "Event (type: PointerMotion, delta: " << delta.unaccelerated() << ")";

    if (m_pressTimeoutTimer.isActive()) {
        qCDebug(INPUTACTIONS_HANDLER_MOUSE, "Event processed (type: PointerMotion, status: PressingButtons)");
        return false;
    }

    m_mouseMotionSinceButtonPress += delta.unacceleratedHypot();
    if (m_mouseMotionSinceButtonPress < 5) {
        qCDebug(INPUTACTIONS_HANDLER_MOUSE).nospace() << "Event processed (type: PointerMotion, status: InsufficientMotion, delta: " << delta.unaccelerated()
                                                      << ")";
        return false;
    }

    // Don't activate triggers if there already had been one since the last press, unless there is an active press trigger, in which case motion should cancel
    // and replace it.
    if ((!m_hadTriggerSincePress || hasActiveTriggers(TriggerType::Press)) && !hasActiveTriggers(TriggerType::All & ~TriggerType::Press)) {
        cancelTriggers(TriggerType::All);
        m_motionTimeoutTimer.stop();

        qCDebug(INPUTACTIONS_HANDLER_MOUSE, "Attempting to activate mouse motion gestures");
        if (!activateTriggers(TriggerType::SinglePointMotion).success) {
            qCDebug(INPUTACTIONS_HANDLER_MOUSE, "No motion gestures");
            pressBlockedMouseButtons(event.sender());
        }
    }

    const auto hadActiveGestures = hasActiveTriggers(TriggerType::SinglePointMotion);
    const auto block = handleMotion(event.sender(), delta);
    if (hadActiveGestures && !hasActiveTriggers(TriggerType::SinglePointMotion)) {
        qCDebug(INPUTACTIONS_HANDLER_MOUSE, "Mouse motion gesture ended/cancelled during motion");
        // Swipe gesture cancelled due to wrong speed or direction
        pressBlockedMouseButtons(event.sender());
    }
    const auto lockPointer = std::ranges::any_of(activeTriggers(TriggerType::SinglePointMotion), [](const auto *trigger) {
        return dynamic_cast<const MotionTrigger *>(trigger)->lockPointer();
    });
    return block && lockPointer;
}

void MouseTriggerHandler::onActivatingTrigger(const Trigger *trigger)
{
    m_hadTriggerSincePress = true;
}

std::unique_ptr<TriggerActivationEvent> MouseTriggerHandler::createActivationEvent() const
{
    auto event = TriggerHandler::createActivationEvent();
    event->setMouseButtons(m_buttons);
    return event;
}

bool MouseTriggerHandler::shouldBlockMouseButton(MouseButton button)
{
    const auto event = createActivationEvent();
    // A partial match is required, not an exact one
    event->setMouseButtons({});
    for (const auto &trigger : triggers(TriggerType::All, *event.get())) {
        const auto buttons = trigger->mouseButtons();
        if (trigger->blockEvents()
            && ((trigger->mouseButtonsExactOrder() && std::ranges::equal(m_buttons, buttons | std::views::take(m_buttons.size())))
                || (!trigger->mouseButtonsExactOrder() && std::ranges::contains(buttons, button)))) {
            qCDebug(INPUTACTIONS_HANDLER_MOUSE).noquote().nospace() << "Mouse button blocked (button: " << button.scanCode() << ", trigger: " << trigger->id() << ")";
            return true;
        }
    }
    return false;
}

void MouseTriggerHandler::pressBlockedMouseButtons(InputDevice *target)
{
    for (const auto &button : m_blockedMouseButtons) {
        target->mouseButton(button, true);
        qCDebug(INPUTACTIONS_HANDLER_MOUSE).nospace() << "Mouse button unblocked (button: " << button.scanCode() << ")";
    }
    m_blockedMouseButtons.clear();
}

}