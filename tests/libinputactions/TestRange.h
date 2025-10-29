#pragma once

#include "Test.h"
#include <libinputactions/Range.h>

namespace libinputactions
{

class TestRange : public Test
{
    Q_OBJECT

private slots:
    void contains_qreal_data();
    void contains_qreal();
};

}