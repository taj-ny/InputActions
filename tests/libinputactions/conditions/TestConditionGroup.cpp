#include "TestConditionGroup.h"
#include "utils.h"
#include <libinputactions/conditions/ConditionGroup.h>

namespace libinputactions
{

void TestConditionGroup::evaluate_data()
{
    QTest::addColumn<ConditionGroupMode>("mode");
    QTest::addColumn<std::vector<std::shared_ptr<Condition>>>("conditions");
    QTest::addColumn<ConditionEvaluationResult>("result");

    std::vector allTrue{TRUE_CONDITION, TRUE_CONDITION};
    std::vector allFalse{FALSE_CONDITION, FALSE_CONDITION};
    std::vector trueFalse{TRUE_CONDITION, FALSE_CONDITION};
    std::vector trueError{TRUE_CONDITION, ERROR_CONDITION};
    std::vector falseError{FALSE_CONDITION, ERROR_CONDITION};
    std::vector errorTrue{ERROR_CONDITION, TRUE_CONDITION};
    std::vector errorFalse{ERROR_CONDITION, FALSE_CONDITION};
    std::vector error{ERROR_CONDITION};

    QTest::newRow("all, allTrue - satisfied") << ConditionGroupMode::All << allTrue << ConditionEvaluationResult::Satisfied;
    QTest::newRow("all, allFalse - not satisfied") << ConditionGroupMode::All << allFalse << ConditionEvaluationResult::NotSatisfied;
    QTest::newRow("all, trueFalse - not satisfied") << ConditionGroupMode::All << trueFalse << ConditionEvaluationResult::NotSatisfied;
    QTest::newRow("all, trueError - error") << ConditionGroupMode::All << trueError << ConditionEvaluationResult::Error;
    QTest::newRow("all, falseError - not satisfied") << ConditionGroupMode::All << falseError << ConditionEvaluationResult::NotSatisfied;
    QTest::newRow("all, errorTrue - error") << ConditionGroupMode::All << errorTrue << ConditionEvaluationResult::Error;
    QTest::newRow("all, errorFalse - error") << ConditionGroupMode::All << errorFalse << ConditionEvaluationResult::Error;
    QTest::newRow("all, error - error") << ConditionGroupMode::All << error << ConditionEvaluationResult::Error;

    QTest::newRow("any, allTrue - satisfied") << ConditionGroupMode::Any << allTrue << ConditionEvaluationResult::Satisfied;
    QTest::newRow("any, allFalse - not satisfied") << ConditionGroupMode::Any << allFalse << ConditionEvaluationResult::NotSatisfied;
    QTest::newRow("any, trueFalse - satisfied") << ConditionGroupMode::Any << trueFalse << ConditionEvaluationResult::Satisfied;
    QTest::newRow("any, trueError - satisfied") << ConditionGroupMode::Any << trueError << ConditionEvaluationResult::Satisfied;
    QTest::newRow("any, falseError - error") << ConditionGroupMode::Any << falseError << ConditionEvaluationResult::Error;
    QTest::newRow("any, errorTrue - satisfied") << ConditionGroupMode::Any << errorTrue << ConditionEvaluationResult::Satisfied;
    QTest::newRow("any, errorFalse - error") << ConditionGroupMode::Any << errorFalse << ConditionEvaluationResult::Error;
    QTest::newRow("any, error - error") << ConditionGroupMode::Any << error << ConditionEvaluationResult::Error;

    QTest::newRow("none, allTrue - not satisfied") << ConditionGroupMode::None << allTrue << ConditionEvaluationResult::NotSatisfied;
    QTest::newRow("none, allFalse - satisfied") << ConditionGroupMode::None << allFalse << ConditionEvaluationResult::Satisfied;
    QTest::newRow("none, trueFalse - not satisfied") << ConditionGroupMode::None << trueFalse << ConditionEvaluationResult::NotSatisfied;
    QTest::newRow("none, trueError - not satisfied") << ConditionGroupMode::None << trueError << ConditionEvaluationResult::NotSatisfied;
    QTest::newRow("none, falseError - error") << ConditionGroupMode::None << falseError << ConditionEvaluationResult::Error;
    QTest::newRow("none, errorTrue - error") << ConditionGroupMode::None << errorTrue << ConditionEvaluationResult::Error;
    QTest::newRow("none, errorFalse - error") << ConditionGroupMode::None << errorFalse << ConditionEvaluationResult::Error;
    QTest::newRow("none, error - error") << ConditionGroupMode::None << error << ConditionEvaluationResult::Error;
}

void TestConditionGroup::evaluate()
{
    QFETCH(ConditionGroupMode, mode);
    QFETCH(std::vector<std::shared_ptr<Condition>>, conditions);
    QFETCH(ConditionEvaluationResult, result);

    ConditionGroup conditionGroup(mode);
    for (const auto &condition : conditions) {
        conditionGroup.add(condition);
    }
    QCOMPARE(conditionGroup.evaluate(), result);
}

}

QTEST_MAIN(libinputactions::TestConditionGroup)
#include "TestConditionGroup.moc"