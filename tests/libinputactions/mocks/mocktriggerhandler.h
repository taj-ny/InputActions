#pragma once

#include <libinputactions/handlers/trigger.h>

#include <gmock/gmock.h>

namespace libinputactions
{

class MockTriggerHandler : public TriggerHandler
{
public:
    MockTriggerHandler() = default;

    MOCK_METHOD(bool, endTriggers, (const TriggerTypes &types, const TriggerEndEvent *event), (override));
    MOCK_METHOD(bool, cancelTriggers, (const TriggerTypes &event), (override));
};

}