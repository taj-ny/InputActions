#pragma once

#include <gmock/gmock.h>
#include <libinputactions/handlers/TouchpadTriggerHandler.h>

namespace InputActions
{

class MockTouchpadTriggerHandler : public TouchpadTriggerHandler
{
public:
    MockTouchpadTriggerHandler(InputDevice *device)
        : TouchpadTriggerHandler(device)
    {
        ON_CALL(*this, handleMotion).WillByDefault([this](const auto *device, const auto &delta) {
            return this->TouchpadTriggerHandler::handleMotion(device, delta);
        });
    }

    MOCK_METHOD(bool, handleMotion, (const InputDevice *device, const PointDelta &delta), (override));
};

}