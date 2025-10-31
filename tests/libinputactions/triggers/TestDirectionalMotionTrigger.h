#pragma once

#include "Test.h"

namespace InputActions
{

class TestDirectionalMotionTrigger : public Test
{
    Q_OBJECT

private slots:
    void canUpdate_data();
    void canUpdate();
};

}