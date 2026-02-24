#pragma once

#include "Test.h"

namespace InputActions
{

class TestActionGroupNodeParser : public Test
{
    Q_OBJECT

private slots:
    void one__parsesNodeCorrectly();
};

}