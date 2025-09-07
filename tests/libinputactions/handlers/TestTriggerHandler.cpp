#include "TestTriggerHandler.h"
#include <QSignalSpy>
#include <linux/input-event-codes.h>

using namespace ::testing;

namespace libinputactions
{

void TestTriggerHandler::init()
{
    m_handler = std::unique_ptr<TriggerHandler>(new TriggerHandler);
}

void TestTriggerHandler::triggers_data()
{
    QTest::addColumn<TriggerType>("type");
    QTest::addColumn<std::vector<Trigger *>>("triggers");
    QTest::addColumn<int>("size");

    QTest::newRow("not activatable") << TriggerType::Press << std::vector<Trigger *>({makeTrigger(TriggerType::Press, false)}) << 0;
    QTest::newRow("activatable") << TriggerType::Press << std::vector<Trigger *>({makeTrigger(TriggerType::Press, true)}) << 1;
    QTest::newRow("activatable, wrong type") << TriggerType::Swipe << std::vector<Trigger *>({makeTrigger(TriggerType::Press, true)}) << 0;
    QTest::newRow("activatable, all") << TriggerType::All
                                      << std::vector<Trigger *>({makeTrigger(TriggerType::Press, true), makeTrigger(TriggerType::Swipe, true)}) << 2;
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

    QCOMPARE(m_handler->triggers(type, event).size(), size);
    QCOMPARE(m_handler->activateTriggers(type, event), size != 0);
    QCOMPARE(m_handler->activeTriggers(type).size(), size);
}

void TestTriggerHandler::activateTriggers_cancelsAllTriggers()
{
    QSignalSpy spy(m_handler.get(), &TriggerHandler::cancellingTriggers);

    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));
    m_handler->activateTriggers(TriggerType::Swipe);
    QCOMPARE(spy.count(), 0);
    m_handler->activateTriggers(TriggerType::Swipe | TriggerType::Press);
    QCOMPARE(spy.count(), 0);
    m_handler->activateTriggers(TriggerType::All);
    QCOMPARE(spy.count(), 1);

    for (const auto &args : spy) {
        QCOMPARE(args.at(0).value<TriggerTypes>(), TriggerType::All);
    }
}

void TestTriggerHandler::keyboardKey()
{
    QSignalSpy spy(m_handler.get(), &TriggerHandler::endingTriggers);
    InputDevice device(InputDeviceType::Keyboard);
    m_handler->addTrigger(std::make_unique<Trigger>(TriggerType::Press));

    m_handler->activateTriggers(TriggerType::Press);
    m_handler->handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, true));
    QCOMPARE(spy.count(), 0);

    m_handler->handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, false));
    QCOMPARE(spy.count(), 1);
}

MockTrigger *TestTriggerHandler::makeTrigger(TriggerType type, bool activatable)
{
    auto *trigger = new MockTrigger(type);
    ON_CALL(*trigger, canActivate(_)).WillByDefault(Return(activatable));
    return trigger;
}

}

QTEST_MAIN(libinputactions::TestTriggerHandler)
#include "TestTriggerHandler.moc"