#pragma once

#include "Test.h"

namespace InputActions
{

class TestMotionTrigger : public Test
{
    Q_OBJECT

private slots:
    void canUpdate_speed_data();
    void canUpdate_speed();
};

}