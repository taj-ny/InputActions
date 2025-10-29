#pragma once

#include "Test.h"

namespace InputActions
{

class TestKeyboardTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void handleEvent_keyboardKey();
};

}