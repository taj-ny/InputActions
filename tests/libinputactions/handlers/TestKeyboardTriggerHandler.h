#pragma once

#include "Test.h"

namespace libinputactions
{

class TestKeyboardTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void handleEvent_keyboardKey();
};

}