#pragma once

#include "Test.h"

namespace InputActions
{

class TestAction : public Test
{
    Q_OBJECT

private slots:
    void canExecute_noLimit();
    void canExecute_withLimit();
};

}