#pragma once

#include "Test.h"

namespace InputActions
{

class TestTriggerAction : public Test
{
    Q_OBJECT

private slots:
    void triggerUpdated_intervals_data();
    void triggerUpdated_intervals();

    void triggerUpdated_mergeable();
};

}