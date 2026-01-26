#pragma once

#include "Test.h"

namespace InputActions
{

class TestInputActionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void keyboardKey_twoSeparate__parsedCorrectly();
    void keyboardKey_twoCombined__parsedCorrectly();
    void keyboardKeyPress_two__parsedCorrectly();
    void keyboardKeyRelease_two__parsedCorrectly();
    void keyboardText__parsedCorrectly();
    void keyboardText_command__parsedCorrectly();

    void mouseAxis__parsedCorrectly();
    void mouseButton_twoSeparate__parsedCorrectly();
    void mouseButton_twoCombined__parsedCorrectly();
    void mouseButtonPress_two__parsedCorrectly();
    void mouseButtonRelease_two__parsedCorrectly();
    void mouseMoveAbsolute__parsedCorrectly();
    void mouseMoveByDelta__parsedCorrectly();
    void mouseMoveRelative__parsedCorrectly();

    void invalid__throwsInvalidValueConfigException_data();
    void invalid__throwsInvalidValueConfigException();
};

}