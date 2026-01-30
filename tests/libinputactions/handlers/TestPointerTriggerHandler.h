#pragma once

#include "Test.h"
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/input/devices/InputDevice.h>

namespace InputActions
{

class TestPointerTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void init();

    void hover_conditionNotSatisfied_triggerNotActivated();
    void hover_conditionSatisfied_triggerActivated();
    void hover_conditionNoLongerSatisfied_triggerEnded();
    void hover_conditionNoLongerSatisfiedNoMotionEvent_triggerEnded();

private:
    std::unique_ptr<InputDevice> m_device;
};

}