#pragma once

#include "Test.h"

namespace InputActions
{

class TestTriggerActionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void valid__doesNotThrow();

    void interval_on_data();
    void interval_on();

    void threshold_on_data();
    void threshold_on();
};

}