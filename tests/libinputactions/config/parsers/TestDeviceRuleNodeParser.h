#pragma once

#include "Test.h"

namespace InputActions
{

class TestDeviceRuleNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__parsesNodeCorrectly();

    void mouse_triggerHandlerSetting_motionTimeout__parsesNodeCorrectly();
    void mouse_triggerHandlerSetting_motionTimeout__addsDeprecatedFeatureConfigIssue();

    void mouse_triggerHandlerSetting_pressTimeout__parsesNodeCorrectly();
    void mouse_triggerHandlerSetting_pressTimeout__addsDeprecatedFeatureConfigIssue();

    void mouse_triggerHandlerSetting_unblockButtonsOnTimeout__parsesNodeCorrectly();
    void mouse_triggerHandlerSetting_unblockButtonsOnTimeout__addsDeprecatedFeatureConfigIssue();

    void touchpad_devicesNode__parsesNodeCorrectly();
    void touchpad_devicesNode__addsDeprecatedFeatureConfigIssue();

    void touchpad_triggerHandlerSetting_clickTimeout__parsesNodeCorrectly();
    void touchpad_triggerHandlerSetting_clickTimeout__addsDeprecatedFeatureConfigIssue();
};

}