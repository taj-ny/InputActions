#pragma once

#include "Test.h"

namespace InputActions
{

class TestInputTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void keyboardKey();
};

}