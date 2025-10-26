#include "TestPointerTriggerHandler.h"
#include "utils.h"
#include <QSignalSpy>
#include <libinputactions/input/events.h>
#include <libinputactions/triggers/HoverTrigger.h>

namespace libinputactions
{

void TestPointerTriggerHandler::init()
{
    m_device = std::make_unique<InputDevice>(InputDeviceType::Mouse);
}

void TestPointerTriggerHandler::hover_conditionNotSatisfied_triggerNotActivated()
{
    auto trigger = std::make_unique<HoverTrigger>();
    trigger->m_activationCondition = FALSE_CONDITION;
    QSignalSpy activatedSpy(trigger.get(), &Trigger::activated);

    PointerTriggerHandler handler;
    handler.addTrigger(std::move(trigger));

    QVERIFY(!handler.handleEvent(MotionEvent(m_device.get(), InputEventType::PointerMotion, {1, 0})));
    QCOMPARE(activatedSpy.count(), 0);
}

void TestPointerTriggerHandler::hover_conditionSatisfied_triggerActivated()
{
    auto trigger = std::make_unique<HoverTrigger>();
    QSignalSpy activatedSpy(trigger.get(), &Trigger::activated);

    PointerTriggerHandler handler;
    handler.addTrigger(std::move(trigger));

    QVERIFY(!handler.handleEvent(MotionEvent(m_device.get(), InputEventType::PointerMotion, {1, 0})));
    QCOMPARE(activatedSpy.count(), 1);
}

void TestPointerTriggerHandler::hover_conditionNoLongerSatisfied_triggerEnded()
{
    auto trigger = std::make_unique<HoverTrigger>();
    auto conditionResult = ConditionEvaluationResult::Satisfied;
    trigger->m_activationCondition = referenceCondition(conditionResult);
    QSignalSpy endedSpy(trigger.get(), &Trigger::ended);

    PointerTriggerHandler handler;
    handler.addTrigger(std::move(trigger));

    QVERIFY(!handler.handleEvent(MotionEvent(m_device.get(), InputEventType::PointerMotion, {1, 0})));
    handler.updateTriggers(TriggerType::Hover);
    QCOMPARE(endedSpy.count(), 0);

    conditionResult = ConditionEvaluationResult::NotSatisfied;
    QVERIFY(!handler.handleEvent(MotionEvent(m_device.get(), InputEventType::PointerMotion, {1, 0})));
    QCOMPARE(endedSpy.count(), 1);
}

void TestPointerTriggerHandler::hover_conditionNoLongerSatisfiedNoMotionEvent_triggerEnded()
{
    auto trigger = std::make_unique<HoverTrigger>();
    auto conditionResult = ConditionEvaluationResult::Satisfied;
    trigger->m_activationCondition = referenceCondition(conditionResult);
    QSignalSpy endedSpy(trigger.get(), &Trigger::ended);

    PointerTriggerHandler handler;
    handler.addTrigger(std::move(trigger));

    QVERIFY(!handler.handleEvent(MotionEvent(m_device.get(), InputEventType::PointerMotion, {1, 0})));
    handler.updateTriggers(TriggerType::Hover);
    QCOMPARE(endedSpy.count(), 0);

    conditionResult = ConditionEvaluationResult::NotSatisfied;
    handler.updateTriggers(TriggerType::Hover);
    QCOMPARE(endedSpy.count(), 1);
}

}

QTEST_MAIN(libinputactions::TestPointerTriggerHandler)
#include "TestPointerTriggerHandler.moc"