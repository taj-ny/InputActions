#pragma once

#include <libinputactions/triggers/trigger.h>

#include <gmock/gmock.h>

namespace libinputactions
{

class MockTrigger : public Trigger
{
public:
    MockTrigger() = default;

    MOCK_METHOD(bool, canActivate, (const TriggerActivationEvent *event), (const, override));
};

}