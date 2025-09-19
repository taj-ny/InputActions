#pragma once

#include <QTest>
#include <libinputactions/handlers/PointerTriggerHandler.h>
#include <libinputactions/input/InputDevice.h>

namespace libinputactions
{

class TestPointerTriggerHandler : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void hover_conditionNotSatisfied_triggerNotActivated();
    void hover_conditionSatisfied_triggerActivated();
    void hover_conditionNoLongerSatisfied_triggerEnded();
    void hover_conditionNoLongerSatisfiedNoMotionEvent_triggerEnded();

private:
    std::unique_ptr<InputDevice> m_device;
};

}