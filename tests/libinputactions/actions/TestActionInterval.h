#pragma once

#include "Test.h"

namespace libinputactions
{

class TestActionInterval : public Test
{
    Q_OBJECT

private slots:
    void matches_data();
    void matches();
};

}