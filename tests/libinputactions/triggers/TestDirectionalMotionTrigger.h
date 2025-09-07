#pragma once

#include <QTest>

namespace libinputactions
{

class TestDirectionalMotionTrigger : public QObject
{
    Q_OBJECT

private slots:
    void canUpdate_data();
    void canUpdate();
};

}