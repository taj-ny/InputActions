#pragma once

#include <QTest>

namespace libinputactions
{

class TestMotionTrigger : public QObject
{
    Q_OBJECT

private slots:
    void canUpdate_speed_data();
    void canUpdate_speed();
};

}