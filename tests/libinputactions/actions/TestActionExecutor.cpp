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
    m_executor->execute(std::make_shared<CustomAction>([]() {
        QVERIFY(isMainThread());
    }));
}

void TestActionExecutor::execute_asyncAction_executedOnActionThread()
{
    m_executor->execute(std::make_shared<CustomAction>(
        []() {
            QVERIFY(!isMainThread());
        },
        true));
    QTest::qWait(100);
}

void TestActionExecutor::execute_syncActionWhileActionThreadIsBusy_executedOnActionThread()
{
    m_executor->execute(std::make_shared<SleepAction>(std::chrono::milliseconds(100U)));
    m_executor->execute(std::make_shared<CustomAction>([]() {
        QVERIFY(!isMainThread());
    }));
    QTest::qWait(500);
}

void TestActionExecutor::execute_syncAndAsyncActions_orderPreserved()
{
    std::vector<uint8_t> results;
    m_executor->execute(std::make_shared<CustomAction>([&results]() {
        results.push_back(1);
    }));
    m_executor->execute(std::make_shared<CustomAction>(
        [&results]() {
            QTest::qWait(20);
            results.push_back(2);
        },
        true));
    m_executor->execute(std::make_shared<CustomAction>([&results]() {
        results.push_back(3);
    }));
    m_executor->execute(std::make_shared<CustomAction>(
        [&results]() {
            QTest::qWait(10);
            results.push_back(4);
        },
        true));
    m_executor->execute(std::make_shared<CustomAction>([&results]() {
        results.push_back(5);
    }));
    QTest::qWait(50);
    QVERIFY((results == std::vector<uint8_t>{1, 2, 3, 4, 5}));
}

bool TestActionExecutor::isMainThread()
{
    return QThread::currentThread() == QCoreApplication::instance()->thread();
}

}

QTEST_MAIN(InputActions::TestActionExecutor)
#include "TestActionExecutor.moc"