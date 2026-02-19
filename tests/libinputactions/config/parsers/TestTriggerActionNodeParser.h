#pragma once

#include "Test.h"

namespace InputActions
{

class TestTriggerActionNodeParser : public Test
{
    Q_OBJECT

private slots:
    void interval_validOn__doesNotThrow_data();
    void interval_validOn__doesNotThrow();
    void interval_invalidOn__throwsInvalidValueConfigException_data();
    void interval_invalidOn__throwsInvalidValueConfigException();

    void threshold_validOn__doesNotThrow_data();
    void threshold_validOn__doesNotThrow();
    void threshold_invalidOn__throwsInvalidValueConfigException_data();
    void threshold_invalidOn__throwsInvalidValueConfigException();
};

}