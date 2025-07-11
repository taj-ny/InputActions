#pragma once

#include <libinputactions/handlers/TriggerHandler.h>

#include "mocks/MockTrigger.h"
#include "mocks/MockTriggerHandler.h"

#include <QTest>

namespace libinputactions
{

class TestTriggerHandler : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void triggers_data();
    void triggers();

    void activateTriggers_cancelsAllTriggers();
    void activateTriggers_invokesCustomHandler();

    void keyboardKey_data();
    void keyboardKey();

private:
    MockTrigger *makeTrigger(TriggerType type, bool activatable);

    std::unique_ptr<MockTriggerHandler> m_handler;
};

}