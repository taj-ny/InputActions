#pragma once

#include <libinputactions/triggers/MotionTrigger.h>

#include <QTest>

namespace libinputactions
{

class TestMotionTrigger : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void canUpdate_data();
    void canUpdate();

    void hasSpeed_data();
    void hasSpeed();

private:
    std::unique_ptr<MotionTrigger> m_motionTrigger;
    MotionTriggerUpdateEvent m_event;
};

}