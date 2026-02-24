#include "TestActionExecutor.h"
#include <libinputactions/actions/CustomAction.h>
#include <libinputactions/actions/SleepAction.h>

namespace InputActions
{

void TestActionExecutor::init()
{
    m_executor = std::make_unique<ActionExecutor>();
}

void TestActionExecutor::execute_syncAction_executedOnMainThread()
{
    CustomAction assertAction([](auto) {
        QCOMPARE(isMainThread(), true);
    });

    m_executor->execute(assertAction);

    QCOMPARE(assertAction.executions(), 1);
}

void TestActionExecutor::execute_asyncAction_executedOnActionThread()
{
    CustomAction assertAction([](auto) {
        QCOMPARE(isMainThread(), false);
    }, true);

    m_executor->execute(assertAction);
    m_executor->waitForDone();

    QCOMPARE(assertAction.executions(), 1);
}

void TestActionExecutor::execute_syncActionWhileActionThreadIsBusy_executedOnActionThread()
{
    SleepAction sleepAction(std::chrono::milliseconds(100U));
    CustomAction assertAction([](auto) {
        QCOMPARE(isMainThread(), false);
    });

    m_executor->execute(sleepAction);
    m_executor->execute(assertAction);
    m_executor->waitForDone();

    QCOMPARE(sleepAction.executions(), 1);
    QCOMPARE(assertAction.executions(), 1);
}

void TestActionExecutor::execute_syncAndAsyncActions_orderPreserved()
{
    std::vector<uint8_t> results;

    CustomAction action1([&results](auto) {
        results.push_back(1);
    });
    CustomAction action2(
        [&results](auto) {
            QTest::qWait(20);
            results.push_back(2);
        },
        true);
    CustomAction action3([&results](auto) {
        results.push_back(3);
    });
    CustomAction action4(
        [&results](auto) {
            QTest::qWait(10);
            results.push_back(4);
        },
        true);
    CustomAction action5([&results](auto) {
        results.push_back(5);
    });

    m_executor->execute(action1);
    m_executor->execute(action2);
    m_executor->execute(action3);
    m_executor->execute(action4);
    m_executor->execute(action5);
    m_executor->waitForDone();

    QCOMPARE(action1.executions(), 1);
    QCOMPARE(action2.executions(), 1);
    QCOMPARE(action3.executions(), 1);
    QCOMPARE(action4.executions(), 1);
    QCOMPARE(action5.executions(), 1);
    QVERIFY((results == std::vector<uint8_t>{1, 2, 3, 4, 5}));
}

bool TestActionExecutor::isMainThread()
{
    return QThread::currentThread() == QCoreApplication::instance()->thread();
}

}

QTEST_MAIN(InputActions::TestActionExecutor)