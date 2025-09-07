#include "TestActionInterval.h"
#include <libinputactions/actions/TriggerAction.h>

namespace libinputactions
{

void TestActionInterval::matches_data()
{
    QTest::addColumn<IntervalDirection>("direction");
    QTest::addColumn<int>("value");
    QTest::addColumn<bool>("result");

    QTest::addRow("any, positive") << IntervalDirection::Any << 1 << true;
    QTest::addRow("any, negative") << IntervalDirection::Any << -1 << true;
    QTest::addRow("any, zero") << IntervalDirection::Any << 0 << true;
    QTest::addRow("positive, positive") << IntervalDirection::Positive << 1 << true;
    QTest::addRow("positive, negative") << IntervalDirection::Positive << -1 << false;
    QTest::addRow("positive, zero") << IntervalDirection::Positive << 0 << false;
    QTest::addRow("negative, positive") << IntervalDirection::Negative << 1 << false;
    QTest::addRow("negative, negative") << IntervalDirection::Negative << -1 << true;
    QTest::addRow("negative, zero") << IntervalDirection::Negative << 0 << false;
}

void TestActionInterval::matches()
{
    QFETCH(IntervalDirection, direction);
    QFETCH(int, value);
    QFETCH(bool, result);

    ActionInterval actionInterval;
    actionInterval.setDirection(direction);

    QCOMPARE(actionInterval.matches(value), result);
}

}

QTEST_MAIN(libinputactions::TestActionInterval)
#include "TestActionInterval.moc"