#pragma once

#include <QTest>

namespace libinputactions
{

class TestConditionGroup : public QObject
{
    Q_OBJECT

private slots:
    void evaluate_data();
    void evaluate();
};

}