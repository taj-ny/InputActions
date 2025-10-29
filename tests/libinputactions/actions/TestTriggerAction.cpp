#include "TestTriggerAction.h"
#include <libinputactions/actions/Action.h>
#include <libinputactions/actions/TriggerAction.h>

namespace InputActions
{

void TestTriggerAction::triggerUpdated_intervals_data()
{
    QTest::addColumn<std::vector<qreal>>("deltas");
    QTest::addColumn<ActionInterval>("interval");
    QTest::addColumn<int>("executions");

    ActionInterval interval{};
    QTest::newRow("zeroes") << std::vector<qreal>{0, 0, 0} << interval << 3;
    interval.setValue(2);
    QTest::newRow("accumulation") << std::vector<qreal>{1, 1, 1, 1} << interval << 2;
    QTest::newRow("multiple executions") << std::vector<qreal>{4, 4} << interval << 4;
    QTest::newRow("direction change (any)") << std::vector<qreal>{-4, 1, -4, 1} << interval << 4;
    interval.setDirection(IntervalDirection::Positive);
    QTest::newRow("direction change (positive)") << std::vector<qreal>{-4, 1, -4, 1} << interval << 0;
    interval.setDirection(IntervalDirection::Negative);
    QTest::newRow("direction change (negative)") << std::vector<qreal>{4, -1, 4, -1} << interval << 0;

    interval.setValue(0);
    interval.setDirection(IntervalDirection::Positive);
    QTest::newRow("no infinite loop") << std::vector<qreal>{1, 0} << interval << 1;
}

void TestTriggerAction::triggerUpdated_intervals()
{
    QFETCH(std::vector<qreal>, deltas);
    QFETCH(ActionInterval, interval);
    QFETCH(int, executions);

    auto action = std::make_unique<TriggerAction>();
    action->m_on = On::Update;
    action->m_interval = interval;
    for (const auto &delta : deltas) {
        action->triggerUpdated(delta, {});
    }

    QCOMPARE(action->action()->m_executions, executions);
}

}

QTEST_MAIN(InputActions::TestTriggerAction)
#include "TestTriggerAction.moc"