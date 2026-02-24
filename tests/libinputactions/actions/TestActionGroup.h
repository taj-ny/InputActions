#pragma once

#include "Test.h"

namespace InputActions
{

class TestActionGroup : public Test
{
    Q_OBJECT

private slots:
    void all_execute__propagatesArgumentsToSubActions();
    void first_execute__propagatesArgumentsToSubActions();
};

}