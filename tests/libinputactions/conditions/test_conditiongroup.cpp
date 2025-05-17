#include "test_conditiongroup.h"

#include "utils.h"

#include <libinputactions/conditions/callbackcondition.h>
#include <libinputactions/conditions/conditiongroup.h>

namespace libinputactions
{

void TestConditionGroup::satisfies_data()
{
    QTest::addColumn<std::vector<bool>>("conditions");
    QTest::addColumn<ConditionGroupMode>("mode");
    QTest::addColumn<bool>("result");

    QTest::newRow("none, none") << std::vector<bool>{} << ConditionGroupMode::None << true;
    QTest::newRow("none, any") << std::vector<bool>{} << ConditionGroupMode::Any << false;
    QTest::newRow("none, all") << std::vector<bool>{} << ConditionGroupMode::All << true;
    QTest::newRow("true, none") << std::vector<bool>{ true } << ConditionGroupMode::None << false;
    QTest::newRow("true, any") << std::vector<bool>{ true } << ConditionGroupMode::Any << true;
    QTest::newRow("true, all") << std::vector<bool>{ true } << ConditionGroupMode::All << true;
    QTest::newRow("false, none") << std::vector<bool>{ false } << ConditionGroupMode::None << true;
    QTest::newRow("false, any") << std::vector<bool>{ false } << ConditionGroupMode::Any << false;
    QTest::newRow("false, all") << std::vector<bool>{ false } << ConditionGroupMode::All << false;
    QTest::newRow("true false, none") << std::vector<bool>{ true, false } << ConditionGroupMode::None << false;
    QTest::newRow("true false, any") << std::vector<bool>{ true, false } << ConditionGroupMode::Any << true;
    QTest::newRow("true false, all") << std::vector<bool>{ true, false } << ConditionGroupMode::All << false;
}

void TestConditionGroup::satisfies()
{
    QFETCH(std::vector<bool>, conditions);
    QFETCH(ConditionGroupMode, mode);
    QFETCH(bool, result);

    ConditionGroup group(mode);
    for (const auto &conditionResult : conditions) {
        group.add(conditionResult ? makeCondition(true) : makeCondition(false));
    }

    QCOMPARE(group.satisfied(), result);
}

}

QTEST_MAIN(libinputactions::TestConditionGroup)
#include "test_conditiongroup.moc"