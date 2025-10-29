#pragma once

#include "Test.h"

namespace libinputactions
{

class TestDirectionalMotionTrigger : public Test
{
    Q_OBJECT

private slots:
    void canUpdate_data();
    void canUpdate();
};

}