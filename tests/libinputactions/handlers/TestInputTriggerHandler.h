#pragma once

#include "Test.h"

namespace libinputactions
{

class TestInputTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void keyboardKey();
};

}