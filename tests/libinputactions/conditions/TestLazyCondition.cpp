#include "TestLazyCondition.h"
#include "utils.h"
#include <libinputactions/conditions/LazyCondition.h>

namespace libinputactions
{

void TestLazyCondition::evaluate()
{
    bool canConstruct{};
    LazyCondition condition([&canConstruct](const auto &arguments) {
        return canConstruct ? TRUE_CONDITION : nullptr;
    });

    QVERIFY_THROWS_EXCEPTION(std::exception, condition.evaluate());
    QVERIFY_THROWS_EXCEPTION(std::exception, condition.evaluate());
    canConstruct = true;
    QVERIFY(condition.evaluate());
    canConstruct = false;
    QVERIFY(condition.evaluate());
}

}

QTEST_MAIN(libinputactions::TestLazyCondition)
#include "TestLazyCondition.moc"