#include "TestTriggerAction.h"
#include <libinputactions/actions/Action.h>
#include <libinputactions/actions/CustomAction.h>
#include <libinputactions/actions/TriggerAction.h>
#include <libinputactions/input/Delta.h>

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
    action->setOn(On::Update);
    action->setInterval(interval);
    for (const auto &delta : deltas) {
        action->triggerUpdated(delta, {});
    }

    QCOMPARE(action->action()->executions(), executions);
}

void TestTriggerAction::triggerUpdated_mergeable()
{
    uint32_t actualExecutions{};
    auto action = std::make_unique<TriggerAction>(std::make_unique<CustomAction>(
        [&actualExecutions](auto executions) {
            actualExecutions = executions;
        },
        false,
        true));

    ActionInterval interval{};
    interval.setValue(1);
    action->setOn(On::Update);
    action->setInterval(interval);

    action->triggerUpdated(10, {});

    QCOMPARE(actualExecutions, 10);
}

}

QTEST_MAIN(InputActions::TestTriggerAction)