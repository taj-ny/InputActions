#pragma once

#include "Test.h"
#include <libinputactions/Range.h>

namespace InputActions
{

class TestRange : public Test
{
    Q_OBJECT

private slots:
    void contains_qreal_data();
    void contains_qreal();
};

}