#pragma once

#include "Test.h"

namespace libinputactions
{

class TestTriggerAction : public Test
{
    Q_OBJECT

private slots:
    void triggerUpdated_intervals_data();
    void triggerUpdated_intervals();
};

}