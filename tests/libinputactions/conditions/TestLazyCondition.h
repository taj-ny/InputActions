#pragma once

#include <QTest>

namespace libinputactions
{

class TestLazyCondition : public QObject
{
    Q_OBJECT

private slots:
    void evaluate();
};

}