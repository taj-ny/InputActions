#pragma once

#include <QTest>

namespace libinputactions
{

class TestKeyboardTriggerHandler : public QObject
{
    Q_OBJECT

private slots:
    void handleEvent_keyboardKey();
};

}