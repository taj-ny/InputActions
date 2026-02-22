#pragma once

#include "Test.h"
#include "mocks/MockMotionTriggerHandler.h"
#include <libinputactions/input/devices/InputDevice.h>

namespace InputActions
{

class TestMotionTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void init();

    void handleMotion_swipe__calculatesAnglesCorrectly_data();
    void handleMotion_swipe__calculatesAnglesCorrectly();

    void handleMotion_swipe__calculatesAverageAngleCorrectly();
    void handleMotion_swipe__motionBeforeThresholdIsTakenIntoAccountWhenCalculatingAverageAngle();

    void handleMotion_swipe_updatedOnceThenCancelled__activatesSwipeTriggers();
    void handleMotion_swipe_cancelledWithoutUpdate__doesNotActivateSwipeTriggers();

private:
    std::unique_ptr<MockMotionTriggerHandler> m_handler;
    std::unique_ptr<InputDevice> m_mouse;
};

}