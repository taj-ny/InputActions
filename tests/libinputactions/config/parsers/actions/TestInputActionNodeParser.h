#pragma once

#include "Test.h"

namespace InputActions
{

class TestInputActionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void keyboardKey_twoSeparate__parsesNodeCorrectly();
    void keyboardKey_twoCombined__parsesNodeCorrectly();
    void keyboardKeyPress_two__parsesNodeCorrectly();
    void keyboardKeyRelease_two__parsesNodeCorrectly();
    void keyboardText__parsesNodeCorrectly();
    void keyboardText_command__parsesNodeCorrectly();

    void mouseButton_twoSeparate__parsesNodeCorrectly();
    void mouseButton_twoCombined__parsesNodeCorrectly();
    void mouseButtonPress_two__parsesNodeCorrectly();
    void mouseButtonRelease_two__parsesNodeCorrectly();
    void mouseMoveAbsolute__parsesNodeCorrectly();
    void mouseMoveByDelta_noMultiplier__parsesNodeCorrectly();
    void mouseMoveByDelta_multiplier__parsesNodeCorrectly();
    void mouseMoveRelative__parsesNodeCorrectly();
    void mouseWheel__parsesNodeCorrectly();

    void invalid__throwsInvalidValueConfigException_data();
    void invalid__throwsInvalidValueConfigException();
};

}