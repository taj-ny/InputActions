#include "test_triggerhandler.h"

#include <linux/input-event-codes.h>

using namespace ::testing;

namespace libinputactions
{

void TestTriggerHandler::init()
{
    m_handler = std::make_unique<MockTriggerHandler>();
}

void TestTriggerHandler::triggers_data()
{
    QTest::addColumn<TriggerType>("type");
    QTest::addColumn<std::vector<Trigger *>>("triggers");
    QTest::addColumn<int>("size");

    QTest::newRow("not activatable")
        << TriggerType::Press
        << std::vector<Trigger *>({makeTrigger(TriggerType::Press, false)})
        << 0;
    QTest::newRow("activatable")
        << TriggerType::Press
        << std::vector<Trigger *>({makeTrigger(TriggerType::Press, true)})
        << 1;
    QTest::newRow("activatable, wrong type")
        << TriggerType::Swipe
        << std::vector<Trigger *>({makeTrigger(TriggerType::Press, true)})
        << 0;
    QTest::newRow("activatable, all")
        << TriggerType::All
        << std::vector<Trigger *>({makeTrigger(TriggerType::Press, true), makeTrigger(TriggerType::Swipe, true)})
        << 2;
}

void TestTriggerHandler::triggers()
{
    QFETCH(TriggerType, type);
    QFETCH(std::vector<Trigger *>, triggers);
    QFETCH(int, size);

    for (auto trigger : triggers) {
        m_handler->addTrigger(std::unique_ptr<Trigger>(trigger));
    }
    TriggerActivationEvent event;

    QCOMPARE(m_handler->triggers(type, &event).size(), size);
    QCOMPARE(m_handler->activateTriggers(type, &event), size != 0);
    QCOMPARE(m_handler->activeTriggers(type).size(), size);
}

void TestTriggerHandler::activateTriggers_cancelsAllTriggers()
{
    EXPECT_CALL(*m_handler, cancelTriggers(static_cast<TriggerTypes>(TriggerType::All)))
        .Times(Exactly(3));

    m_handler->activateTriggers(TriggerType::Swipe);
    m_handler->activateTriggers(TriggerType::Swipe | TriggerType::Press);
    m_handler->activateTriggers(TriggerType::All);

    QVERIFY(Mock::VerifyAndClearExpectations(m_handler.get()));
}

void TestTriggerHandler::activateTriggers_invokesCustomHandler()
{
    uint32_t swipe{};
    uint32_t press{};
    m_handler->registerTriggerActivateHandler(TriggerType::Swipe, [&swipe] { swipe++; });
    m_handler->registerTriggerActivateHandler(TriggerType::Press, [&press] { press++; });

    m_handler->activateTriggers(TriggerType::All);
    QCOMPARE(swipe, 1);
    QCOMPARE(press, 1);

    m_handler->activateTriggers(TriggerType::Swipe);
    QCOMPARE(swipe, 2);
    QCOMPARE(press, 1);

    m_handler->activateTriggers(TriggerType::Swipe | TriggerType::Press);
    QCOMPARE(swipe, 3);
    QCOMPARE(press, 2);
}

void TestTriggerHandler::keyboardKey_data()
{
    QTest::addColumn<int>("key");
    QTest::addColumn<bool>("state");
    QTest::addColumn<bool>("endsTriggers");

    QTest::newRow("press") << KEY_LEFTCTRL << true << false;
    QTest::newRow("release") << KEY_LEFTCTRL << false << true;
}

void TestTriggerHandler::keyboardKey()
{
    QFETCH(int, key);
    QFETCH(bool, state);
    QFETCH(bool, endsTriggers);

    EXPECT_CALL(*m_handler, endTriggers(static_cast<TriggerTypes>(TriggerType::All)))
        .Times(Exactly(endsTriggers ? 1 : 0));

    InputDevice device(InputDeviceType::Keyboard);
    const KeyboardKeyEvent event(&device, key, state);
    m_handler->handleEvent(&event);

    QVERIFY(Mock::VerifyAndClearExpectations(m_handler.get()));
}

MockTrigger *TestTriggerHandler::makeTrigger(const TriggerType &type, const bool &activatable)
{
    auto *trigger = new MockTrigger;
    trigger->setType(type);
    ON_CALL(*trigger, canActivate(_))
        .WillByDefault(Return(activatable));
    return trigger;
}

}

QTEST_MAIN(libinputactions::TestTriggerHandler)
#include "test_triggerhandler.moc"