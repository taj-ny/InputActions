#pragma once

#include <QTest>

namespace libinputactions
{

class TestInputTriggerHandler : public QObject
{
    Q_OBJECT

private slots:
    void keyboardKey();
};

}