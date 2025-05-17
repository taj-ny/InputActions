#include "test_trigger.h"

#include <libinputactions/conditions/callbackcondition.h>
#include <libinputactions/conditions/conditiongroup.h>

#include <QSignalSpy>

using namespace ::testing;

namespace libinputactions
{

void TestTrigger::init()
{
    m_trigger = std::make_unique<Trigger>();
    m_activationEvent = std::make_unique<TriggerActivationEvent>();
    m_updateEvent = std::make_unique<TriggerUpdateEvent>();
    m_action = new MockGestureAction; // Small memory leak, sometimes gets deleted by Trigger, sometimes not
}

void TestTrigger::canActivate_data()
{
    QTest::addColumn<Trigger *>("trigger");
    QTest::addColumn<TriggerActivationEvent *>("event");
    QTest::addColumn<bool>("result");


    auto trigger = new Trigger;
    auto event = new TriggerActivationEvent;
    QTest::newRow("nothing") << trigger << event << true;
    trigger = new Trigger;
    event = new TriggerActivationEvent;
    trigger->setActivationCondition(std::make_shared<CallbackCondition>([]() { return false; }));
    QTest::newRow("false condition") << trigger << event << false;
    trigger = new Trigger;
    event = new TriggerActivationEvent;
    trigger->setActivationCondition(std::make_shared<CallbackCondition>([]() { return true; }));
    QTest::newRow("true condition") << trigger << event << true;

    const auto mouseButton = Qt::MouseButton::LeftButton;
    trigger = new Trigger;
    event = new TriggerActivationEvent;
    trigger->setMouseButtons(mouseButton);
    event->mouseButtons = mouseButton;
    QTest::newRow("mouse buttons, both, correct") << trigger << event << true;
    trigger = new Trigger;
    event = new TriggerActivationEvent;
    trigger->setMouseButtons(mouseButton);
    event->mouseButtons = Qt::MouseButton::RightButton;
    QTest::newRow("mouse buttons, both, wrong") << trigger << event << false;
    trigger = new Trigger;
    event = new TriggerActivationEvent;
    trigger->setMouseButtons(mouseButton);
    QTest::newRow("mouse buttons, trigger only") << trigger << event << true;
    trigger = new Trigger;
    event = new TriggerActivationEvent;
    event->mouseButtons = mouseButton;
    QTest::newRow("mouse buttons, event only") << trigger << event << true;
}

void TestTrigger::canActivate()
{
    QFETCH(Trigger *, trigger);
    QFETCH(TriggerActivationEvent *, event);
    QFETCH(bool, result);

    QCOMPARE(trigger->canActivate(event), result);

    delete trigger;
    delete event;
}

void TestTrigger::update_data()
{
    QTest::addColumn<std::optional<Range<qreal>>>("threshold");
    QTest::addColumn<std::vector<qreal>>("deltas");
    QTest::addColumn<bool>("actionExecuted");

    // total delta > threshold is tested to ensure the begin action only executes once
    QTest::newRow("no threshold, 0 delta") << std::optional<Range<qreal>>() << std::vector<qreal>{0} << true;
    QTest::newRow("total delta < min_threshold") << std::optional<Range<qreal>>(Range<qreal>(3, {})) << std::vector<qreal>{1, 1} << false;
    QTest::newRow("total delta > min_threshold") << std::optional<Range<qreal>>(Range<qreal>(3, {})) << std::vector<qreal>{2, 2} << true;
    QTest::newRow("delta1, delta2 > max_threshold") << std::optional<Range<qreal>>(Range<qreal>({}, 3))  << std::vector<qreal>{4, 4} << false;
    QTest::newRow("total delta > max_threshold") << std::optional<Range<qreal>>(Range<qreal>({}, 3)) << std::vector<qreal>{2, 2}  << true;
}

void TestTrigger::update()
{
    QFETCH(std::optional<Range<qreal>>, threshold);
    QFETCH(std::vector<qreal>, deltas);
    QFETCH(bool, actionExecuted);

    m_action->setOn(On::Begin);
    m_trigger->addAction(std::unique_ptr<TriggerAction>(m_action));
    if (threshold) {
        m_trigger->setThreshold(*threshold);
    }
    EXPECT_CALL(*m_action, execute()).Times(testing::Exactly(actionExecuted ? 1 : 0));

    for (const auto &delta : deltas) {
        m_updateEvent->setDelta(delta);
        m_trigger->update(m_updateEvent.get());
    }
}

void TestTrigger::end_started_informsActionProperly()
{
    EXPECT_CALL(*m_action, triggerStarted()).Times(Exactly(1));
    EXPECT_CALL(*m_action, triggerUpdated(_, _)).Times(Exactly(1));
    EXPECT_CALL(*m_action, triggerEnded()).Times(Exactly(1));
    EXPECT_CALL(*m_action, triggerCancelled()).Times(Exactly(0));

    m_trigger->addAction(std::unique_ptr<TriggerAction>(m_action));
    m_updateEvent->setDelta(0);

    m_trigger->update(m_updateEvent.get());
    m_trigger->end();

    QVERIFY(Mock::VerifyAndClearExpectations(m_action));
}

void TestTrigger::end_notStarted_doesntInformActions()
{
    EXPECT_CALL(*m_action, triggerStarted()).Times(Exactly(0));
    EXPECT_CALL(*m_action, triggerUpdated(_, _)).Times(Exactly(0));
    EXPECT_CALL(*m_action, triggerEnded()).Times(Exactly(0));
    EXPECT_CALL(*m_action, triggerCancelled()).Times(Exactly(0));

    m_action->setOn(On::End);
    m_trigger->addAction(std::unique_ptr<TriggerAction>(m_action));

    m_trigger->end();

    QVERIFY(Mock::VerifyAndClearExpectations(m_action));
}

void TestTrigger::cancel_started_informsActionProperly()
{
    EXPECT_CALL(*m_action, triggerStarted()).Times(Exactly(1));
    EXPECT_CALL(*m_action, triggerUpdated(_, _)).Times(Exactly(1));
    EXPECT_CALL(*m_action, triggerEnded()).Times(Exactly(0));
    EXPECT_CALL(*m_action, triggerCancelled()).Times(Exactly(1));

    m_trigger->addAction(std::unique_ptr<TriggerAction>(m_action));
    m_updateEvent->setDelta(0);

    m_trigger->update(m_updateEvent.get());
    m_trigger->cancel();

    QVERIFY(Mock::VerifyAndClearExpectations(m_action));
}

void TestTrigger::cancel_notStarted_doesntInformActions()
{
    EXPECT_CALL(*m_action, triggerStarted()).Times(Exactly(0));
    EXPECT_CALL(*m_action, triggerUpdated(_, _)).Times(Exactly(0));
    EXPECT_CALL(*m_action, triggerEnded()).Times(Exactly(0));
    EXPECT_CALL(*m_action, triggerCancelled()).Times(Exactly(0));

    m_action->setOn(On::Cancel);
    m_trigger->addAction(std::unique_ptr<TriggerAction>(m_action));

    m_trigger->cancel();

    QVERIFY(Mock::VerifyAndClearExpectations(m_action));
}

void TestTrigger::overridesOtherTriggersOnEnd_data()
{
    QTest::addColumn<On>("on");
    QTest::addColumn<bool>("thresholdReached");
    QTest::addColumn<bool>("canExecute");
    QTest::addColumn<bool>("result");

    QTest::newRow("1") << On::Begin << true << true << false;
    QTest::newRow("2") << On::Update << true << true << false;
    QTest::newRow("3") << On::Cancel << true << true << false;
    QTest::newRow("4") << On::End << true << true << true;
    QTest::newRow("5") << On::EndCancel << true << true << true;
    QTest::newRow("6") << On::End << false << true << false;
    QTest::newRow("7") << On::End << false << true << false;
    QTest::newRow("8") << On::End << false << false << false;
}

void TestTrigger::overridesOtherTriggersOnEnd()
{
    QFETCH(On, on);
    QFETCH(bool, thresholdReached);
    QFETCH(bool, canExecute);
    QFETCH(bool, result);

    ON_CALL(*m_action, canExecute())
        .WillByDefault(::testing::Return(canExecute));

    m_action->setOn(on);
    m_trigger->addAction(std::unique_ptr<TriggerAction>(m_action));
    if (!thresholdReached) {
        m_trigger->setThreshold(Range<qreal>(2, {}));
    }
    m_updateEvent->setDelta(1);

    m_trigger->update(m_updateEvent.get());

    QCOMPARE(m_trigger->overridesOtherTriggersOnEnd(), result);
}

void TestTrigger::overridesOtherTriggersOnEnd_noActions_false()
{
    m_updateEvent->setDelta(1);
    m_trigger->update(m_updateEvent.get());

    QCOMPARE(m_trigger->overridesOtherTriggersOnEnd(), false);
}

void TestTrigger::overridesOtherTriggersOnEnd_noUpdate_false()
{
    m_trigger->addAction(std::unique_ptr<TriggerAction>(m_action));

    QCOMPARE(m_trigger->overridesOtherTriggersOnEnd(), false);
}

void TestTrigger::overridesOtherTriggersOnUpdate_data()
{
    QTest::addColumn<On>("on");
    QTest::addColumn<bool>("thresholdReached");
    QTest::addColumn<bool>("canExecute");
    QTest::addColumn<bool>("executed");
    QTest::addColumn<bool>("result");

    QTest::newRow("1") << On::Begin << true << true << true << true;
    QTest::newRow("2") << On::Begin << true << true << false << false;
    QTest::newRow("3") << On::Begin << true << false << false << false;
    QTest::newRow("4") << On::Begin << false << true << true << false;
    QTest::newRow("5") << On::Update << true << true << true << true;
    QTest::newRow("6") << On::Update << true << true << false << true;
    QTest::newRow("7") << On::Update << true << false << false << false;
    QTest::newRow("8") << On::Update << false << true << true << false;
}

void TestTrigger::overridesOtherTriggersOnUpdate()
{
    QFETCH(On, on);
    QFETCH(bool, thresholdReached);
    QFETCH(bool, canExecute);
    QFETCH(bool, executed);
    QFETCH(bool, result);

    ON_CALL(*m_action, canExecute())
        .WillByDefault(::testing::Return(canExecute));
    ON_CALL(*m_action, executed())
        .WillByDefault(::testing::ReturnRef(executed));

    m_action->setOn(on);
    m_trigger->addAction(std::unique_ptr<TriggerAction>(m_action));
    if (!thresholdReached) {
        m_trigger->setThreshold(Range<qreal>(2, {}));
    }
    m_updateEvent->setDelta(1);

    m_trigger->update(m_updateEvent.get());

    QCOMPARE(m_trigger->overridesOtherTriggersOnUpdate(), result);
}

void TestTrigger::overridesOtherTriggersOnUpdate_noActions_false()
{
    m_updateEvent->setDelta(1);

    m_trigger->update(m_updateEvent.get());

    QCOMPARE(m_trigger->overridesOtherTriggersOnUpdate(), false);
}

void TestTrigger::overridesOtherTriggersOnUpdate_noUpdate_false()
{
    m_trigger->addAction(std::unique_ptr<TriggerAction>(m_action));

    QCOMPARE(m_trigger->overridesOtherTriggersOnUpdate(), false);
}

}

QTEST_MAIN(libinputactions::TestTrigger)
#include "test_trigger.moc"