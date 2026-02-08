#pragma once

#include "Test.h"
#include <libinputactions/handlers/InputTriggerHandler.h>
#include <libinputactions/input/devices/InputDevice.h>

namespace InputActions
{

class TestInputTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void init();

    void keyboardKey__modifierReleased_pressedBeforeTriggerActivation__triggersEnded();
    void keyboardKey__modifierReleased_pressedAfterTriggerActivation__triggersNotEnded();

private:
    std::unique_ptr<InputDevice> m_keyboard;
    std::unique_ptr<InputTriggerHandler> m_handler;
};

}