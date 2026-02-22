#pragma once

#include <gmock/gmock.h>
#include <libinputactions/handlers/MotionTriggerHandler.h>

namespace InputActions
{

class MockMotionTriggerHandler : public MotionTriggerHandler
{
public:
    MockMotionTriggerHandler()
    {
        ON_CALL(*this, activateTriggers).WillByDefault([this](auto types, const auto &event) {
            return this->MotionTriggerHandler::activateTriggers(types, event);
        });
        ON_CALL(*this, updateTriggers).WillByDefault([this](const auto &events) {
            return this->MotionTriggerHandler::updateTriggers(events);
        });
        ON_CALL(*this, hasActiveTriggers).WillByDefault([this](auto triggers) {
            return this->MotionTriggerHandler::hasActiveTriggers(triggers);
        });
    }

    MOCK_METHOD(TriggerManagementOperationResult, activateTriggers, (TriggerTypes types, const TriggerActivationEvent &event), (override));
    MOCK_METHOD(TriggerManagementOperationResult, updateTriggers, ((const std::map<TriggerType, const TriggerUpdateEvent *> &events)), (override));
    MOCK_METHOD(bool, hasActiveTriggers, (TriggerTypes types), (override));
};

}