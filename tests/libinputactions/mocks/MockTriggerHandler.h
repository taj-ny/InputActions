#pragma once

#include <libinputactions/handlers/TriggerHandler.h>

#include <gmock/gmock.h>

namespace libinputactions
{

class MockTriggerHandler : public TriggerHandler
{
public:
    MockTriggerHandler() = default;

    MOCK_METHOD(bool, endTriggers, (TriggerTypes types), (override));
    MOCK_METHOD(bool, cancelTriggers, (TriggerTypes event), (override));
};

}