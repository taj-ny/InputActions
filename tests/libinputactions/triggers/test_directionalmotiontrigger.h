#pragma once

#include <libinputactions/triggers/directionalmotion.h>

#include <QTest>

namespace libinputactions
{

class TestDirectionalMotionTrigger : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void canUpdate_data();
    void canUpdate();

private:
    std::unique_ptr<DirectionalMotionTrigger> m_motionTrigger;
};

}