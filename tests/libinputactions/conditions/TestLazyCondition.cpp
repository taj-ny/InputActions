#include "TestLazyCondition.h"
#include "utils.h"
#include <libinputactions/conditions/LazyCondition.h>

namespace libinputactions
{

void TestLazyCondition::evaluate()
{
    bool canConstruct{};
    LazyCondition condition([&canConstruct]() {
        return canConstruct ? TRUE_CONDITION : nullptr;
    });

    QCOMPARE(condition.evaluate(), ConditionEvaluationResult::Error);
    QCOMPARE(condition.evaluate(), ConditionEvaluationResult::Error);
    canConstruct = true;
    QCOMPARE(condition.evaluate(), ConditionEvaluationResult::Satisfied);
    canConstruct = false;
    QCOMPARE(condition.evaluate(), ConditionEvaluationResult::Satisfied);
}

}

QTEST_MAIN(libinputactions::TestLazyCondition)
#include "TestLazyCondition.moc"