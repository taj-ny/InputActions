#pragma once

#include "mocks/MockTriggerAction.h"

#include <QTest>

namespace libinputactions
{

class TestTriggerAction : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void canExecute_data();
    void canExecute();

    void tryExecute_canExecute_executes();
    void tryExecute_cantExecute_doesntExecute();

    void gestureStarted_data();
    void gestureStarted();

    void gestureUpdated_data();
    void gestureUpdated();

    void gestureEnded_data();
    void gestureEnded();

    void gestureCancelled_data();
    void gestureCancelled();
private:
    std::unique_ptr<MockTriggerAction> m_action;
};

}