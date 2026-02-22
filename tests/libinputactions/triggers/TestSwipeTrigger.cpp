#include "TestSwipeTrigger.h"
#include "mocks/MockSwipeTrigger.h"

using namespace ::testing;

namespace InputActions
{

void TestSwipeTrigger::canUpdate_currentAngleDoesNotMatchRange_averageAngleMatchesRange__returnsTrue()
{
    SwipeTrigger trigger(30, 60);

    SwipeTriggerUpdateEvent event;
    event.setAngle(0);
    event.setAverageAngle(45);

    QCOMPARE(trigger.canUpdate(event), true);
}

void TestSwipeTrigger::canUpdate_currentAngleMatchesRange_averageAngleDoesNotMatchRange__returnsFalse()
{
    SwipeTrigger trigger(30, 60);

    SwipeTriggerUpdateEvent event;
    event.setAngle(45);
    event.setAverageAngle(0);

    QCOMPARE(trigger.canUpdate(event), false);
}

void TestSwipeTrigger::canUpdate_data()
{
    QTest::addColumn<qreal>("min");
    QTest::addColumn<qreal>("max");
    QTest::addColumn<bool>("bidirectional");
    QTest::addColumn<qreal>("angle");
    QTest::addColumn<bool>("returnValue");

    QTest::addRow("normalAngle__returnsTrue") << 30.0 << 60.0 << false << 45.0 << true;
    QTest::addRow("oppositeAngle__returnsFalse") << 30.0 << 60.0 << false << 225.0 << false;
    QTest::addRow("normalAngle_bidirectional__returnsTrue") << 30.0 << 60.0 << true << 45.0 << true;
    QTest::addRow("oppositeAngle_bidirectional__returnsFalse") << 30.0 << 60.0 << true << 225.0 << true;
}

void TestSwipeTrigger::canUpdate()
{
    QFETCH(qreal, min);
    QFETCH(qreal, max);
    QFETCH(bool, bidirectional);
    QFETCH(qreal, angle);
    QFETCH(bool, returnValue);

    SwipeTrigger trigger(min, max);
    trigger.setBidirectional(bidirectional);

    SwipeTriggerUpdateEvent event;
    event.setAverageAngle(angle);

    QCOMPARE(trigger.canUpdate(event), returnValue);
}

void TestSwipeTrigger::updateActions_bidirectional_normalAngle__updatesActionsWithPositiveDelta()
{
    MockSwipeTrigger trigger(0, 0);
    trigger.setBidirectional(true);
    EXPECT_CALL(trigger, doUpdateActions(Property(&TriggerUpdateEvent::delta, Property(&Delta::unaccelerated, Eq(10))))).Times(1);

    SwipeTriggerUpdateEvent event;
    event.setAngle(0);
    event.setDelta(10);

    trigger.updateActions(event);

    QVERIFY(Mock::VerifyAndClearExpectations(&trigger));
}

void TestSwipeTrigger::updateActions_bidirectional_oppositeAngle__updatesActionsWithNegativeDelta()
{
    MockSwipeTrigger trigger(0, 0);
    trigger.setBidirectional(true);
    EXPECT_CALL(trigger, doUpdateActions(Property(&TriggerUpdateEvent::delta, Property(&Delta::unaccelerated, Eq(-10))))).Times(1);

    SwipeTriggerUpdateEvent event;
    event.setAngle(180);
    event.setDelta(10);

    trigger.updateActions(event);

    QVERIFY(Mock::VerifyAndClearExpectations(&trigger));
}

void TestSwipeTrigger::updateActions_bidirectional_overlappingAngleRanges__normalRangeHasHigherPriority()
{
    MockSwipeTrigger trigger(0, 270);
    trigger.setBidirectional(true);
    EXPECT_CALL(trigger, doUpdateActions(Property(&TriggerUpdateEvent::delta, Property(&Delta::unaccelerated, Gt(-10))))).Times(2);

    SwipeTriggerUpdateEvent event;
    event.setAngle(30);
    event.setDelta(10);
    trigger.updateActions(event);

    event.setAngle(260);
    trigger.updateActions(event);

    QVERIFY(Mock::VerifyAndClearExpectations(&trigger));
}

void TestSwipeTrigger::matchesAngleRange_data()
{
    QTest::addColumn<qreal>("a");
    QTest::addColumn<qreal>("b");
    QTest::addColumn<qreal>("angle");
    QTest::addColumn<bool>("result");

    QTest::addRow("a<b, middle => true") << 90.0 << 270.0 << 180.0 << true;
    QTest::addRow("a<b, min => true") << 90.0 << 270.0 << 90.0 << true;
    QTest::addRow("a<b, max => true") << 90.0 << 270.0 << 270.0 << true;
    QTest::addRow("a<b, min-1 => false") << 90.0 << 270.0 << 89.0 << false;
    QTest::addRow("a<b, max+1 => false") << 90.0 << 270.0 << 271.0 << false;
    QTest::addRow("a<b, middle opposite => false") << 90.0 << 270.0 << 0.0 << false;

    QTest::addRow("a>b, middle => true") << 270.0 << 90.0 << 0.0 << true;
    QTest::addRow("a>b, min => true") << 270.0 << 90.0 << 270.0 << true;
    QTest::addRow("a>b, max => true") << 270.0 << 90.0 << 90.0 << true;
    QTest::addRow("a>b, min-1 => false") << 270.0 << 90.0 << 269.0 << false;
    QTest::addRow("a>b, max+1 => false") << 270.0 << 90.0 << 91.0 << false;
    QTest::addRow("a>b, middle opposite => false") << 270.0 << 90.0 << 180.0 << false;

    QTest::addRow("a=b => true") << 0.0 << 0.0 << 0.0 << true;
    QTest::addRow("a=b, opposite => false") << 0.0 << 0.0 << 180.0 << false;
}

void TestSwipeTrigger::matchesAngleRange()
{
    QFETCH(qreal, a);
    QFETCH(qreal, b);
    QFETCH(qreal, angle);
    QFETCH(bool, result);

    SwipeTrigger trigger(a, b);

    QCOMPARE(trigger.matchesAngleRange(angle), result);
}

void TestSwipeTrigger::matchesOppositeAngleRange_data()
{
    QTest::addColumn<qreal>("a");
    QTest::addColumn<qreal>("b");
    QTest::addColumn<qreal>("angle");
    QTest::addColumn<bool>("result");

    QTest::addRow("a<b, middle opposite => true") << 90.0 << 270.0 << 0.0 << true;
    QTest::addRow("a<b, min opposite => true") << 90.0 << 270.0 << 270.0 << true;
    QTest::addRow("a<b, max opposite => true") << 90.0 << 270.0 << 90.0 << true;
    QTest::addRow("a<b, min-1 opposite => false") << 90.0 << 270.0 << 269.0 << false;
    QTest::addRow("a<b, max+1 opposite => false") << 90.0 << 270.0 << 91.0 << false;
    QTest::addRow("a<b, middle normal => false") << 90.0 << 270.0 << 180.0 << false;

    QTest::addRow("a>b, middle opposite => true") << 270.0 << 90.0 << 180.0 << true;
    QTest::addRow("a>b, min opposite => true") << 270.0 << 90.0 << 90.0 << true;
    QTest::addRow("a>b, max opposite => true") << 270.0 << 90.0 << 270.0 << true;
    QTest::addRow("a>b, min-1 opposite => false") << 270.0 << 90.0 << 89.0 << false;
    QTest::addRow("a>b, max+1 opposite => false") << 270.0 << 90.0 << 271.0 << false;
    QTest::addRow("a>b, middle normal => false") << 270.0 << 90.0 << 0.0 << false;

    QTest::addRow("a=b, opposite => true") << 0.0 << 0.0 << 180.0 << true;
    QTest::addRow("a=b, normal => false") << 0.0 << 0.0 << 0.0 << false;
}

void TestSwipeTrigger::matchesOppositeAngleRange()
{
    QFETCH(qreal, a);
    QFETCH(qreal, b);
    QFETCH(qreal, angle);
    QFETCH(bool, result);

    SwipeTrigger trigger(a, b);

    QCOMPARE(trigger.matchesOppositeAngleRange(angle), result);
}

}

QTEST_MAIN(InputActions::TestSwipeTrigger)