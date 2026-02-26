#pragma once

#include <gmock/gmock.h>
#include <libinputactions/triggers/SwipeTrigger.h>

namespace InputActions
{

class MockSwipeTrigger : public SwipeTrigger
{
public:
    MockSwipeTrigger(qreal angleMin, qreal angleMax)
        : SwipeTrigger(angleMin, angleMax)
    {
        ON_CALL(*this, doUpdateActions).WillByDefault([this](const auto &event) {
            this->SwipeTrigger::doUpdateActions(event);
        });
    }

    MOCK_METHOD(void, doUpdateActions, (const TriggerUpdateEvent &event), (override));
};

}