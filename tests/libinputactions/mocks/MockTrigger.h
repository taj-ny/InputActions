#pragma once

#include <libinputactions/triggers/Trigger.h>

#include <gmock/gmock.h>

namespace libinputactions
{

class MockTrigger : public Trigger
{
public:
    MockTrigger(TriggerType type = TriggerType::None)
        : Trigger(type)
    {
    }

    MOCK_METHOD(bool, canActivate, (const TriggerActivationEvent *event), (const, override));
};

}