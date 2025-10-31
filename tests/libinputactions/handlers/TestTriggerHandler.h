#pragma once

#include "Test.h"
#include "mocks/MockTrigger.h"
#include <libinputactions/handlers/TriggerHandler.h>

namespace InputActions
{

class TestTriggerHandler : public Test
{
    Q_OBJECT

private slots:
    void init();

    void triggers_data();
    void triggers();

    void activateTriggers_cancelsAllTriggers();

private:
    MockTrigger *makeTrigger(TriggerType type, bool activatable);

    std::unique_ptr<TriggerHandler> m_handler;
};

}