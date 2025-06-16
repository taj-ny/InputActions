#pragma once

#include "mocks/MockTriggerAction.h"

#include <libinputactions/triggers/Trigger.h>

#include <QTest>

namespace libinputactions
{

class TestTrigger : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void canActivate_data();
    void canActivate();

    void update_data();
    void update();

    void end_started_informsActionProperly();
    void end_notStarted_doesntInformActions();

    void cancel_started_informsActionProperly();
    void cancel_notStarted_doesntInformActions();

    void overridesOtherTriggersOnEnd_data();
    void overridesOtherTriggersOnEnd();
    void overridesOtherTriggersOnEnd_noActions_false();
    void overridesOtherTriggersOnEnd_noUpdate_false();

    void overridesOtherTriggersOnUpdate_data();
    void overridesOtherTriggersOnUpdate();
    void overridesOtherTriggersOnUpdate_noActions_false();
    void overridesOtherTriggersOnUpdate_noUpdate_false();

private:
    std::unique_ptr<Trigger> m_trigger;
    std::unique_ptr<TriggerActivationEvent> m_activationEvent;
    std::unique_ptr<TriggerUpdateEvent> m_updateEvent;
    MockTriggerAction *m_action;
};

}