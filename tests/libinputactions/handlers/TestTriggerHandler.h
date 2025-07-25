#pragma once

#include <libinputactions/handlers/TriggerHandler.h>

#include "mocks/MockTrigger.h"

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

    void keyboardKey();

private:
    MockTrigger *makeTrigger(TriggerType type, bool activatable);

    std::unique_ptr<TriggerHandler> m_handler;
};

}