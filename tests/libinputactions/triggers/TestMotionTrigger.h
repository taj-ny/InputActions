#pragma once

#include "Test.h"

namespace libinputactions
{

class TestMotionTrigger : public Test
{
    Q_OBJECT

private slots:
    void canUpdate_speed_data();
    void canUpdate_speed();
};

}