#include "TestKeyboardTriggerHandler.h"
#include <QSignalSpy>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/triggers/KeyboardShortcutTrigger.h>
#include <linux/input-event-codes.h>

namespace libinputactions
{

void TestKeyboardTriggerHandler::init()
{
    m_device = std::make_unique<InputDevice>(InputDeviceType::Keyboard);
}

void TestKeyboardTriggerHandler::shortcut_oneModifierKeyShortcut_triggerActivatedEndedAndEventsNotBlocked()
{
    auto trigger = std::make_unique<KeyboardShortcutTrigger>(KeyboardShortcut{
        .keys = {KEY_LEFTMETA},
    });
    QSignalSpy activatedSpy(trigger.get(), &Trigger::activated);
    QSignalSpy endedSpy(trigger.get(), &Trigger::ended);

    KeyboardTriggerHandler handler;
    handler.addTrigger(std::move(trigger));

    QVERIFY(!handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_LEFTMETA, true)));
    handler.updateTriggers(TriggerType::KeyboardShortcut);
    QCOMPARE(activatedSpy.count(), 1);
    QCOMPARE(endedSpy.count(), 0);

    QVERIFY(!handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_LEFTMETA, false)));
    QCOMPARE(endedSpy.count(), 1);
}

void TestKeyboardTriggerHandler::shortcut_oneNonModifierKeyShortcut_triggerActivatedEndedAndEventsBlocked()
{
    auto trigger = std::make_unique<KeyboardShortcutTrigger>(KeyboardShortcut{
        .keys = {KEY_A},
    });
    QSignalSpy activatedSpy(trigger.get(), &Trigger::activated);
    QSignalSpy endedSpy(trigger.get(), &Trigger::ended);

    KeyboardTriggerHandler handler;
    handler.addTrigger(std::move(trigger));

    QVERIFY(handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_A, true)));
    handler.updateTriggers(TriggerType::KeyboardShortcut);
    QCOMPARE(activatedSpy.count(), 1);
    QCOMPARE(endedSpy.count(), 0);

    QVERIFY(handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_A, false)));
    QCOMPARE(endedSpy.count(), 1);
}

void TestKeyboardTriggerHandler::shortcut_twoKeysWrongOrder_triggerNotActivatedAndEventsNotBlocked()
{
    auto trigger = std::make_unique<KeyboardShortcutTrigger>(KeyboardShortcut{
        .keys = {KEY_LEFTCTRL, KEY_A},
    });
    QSignalSpy activatedSpy(trigger.get(), &Trigger::activated);

    KeyboardTriggerHandler handler;
    handler.addTrigger(std::move(trigger));

    QVERIFY(!handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_A, true)));
    QVERIFY(!handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_LEFTCTRL, true)));
    QVERIFY(!handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_LEFTCTRL, false)));
    QVERIFY(!handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_A, false)));
    QCOMPARE(activatedSpy.count(), 0);
}

void TestKeyboardTriggerHandler::shortcut_twoKeysCorrectOrder_triggerActivatedAndNormalKeyBlockedAndModifierKeyNotBlocked()
{
    auto trigger = std::make_unique<KeyboardShortcutTrigger>(KeyboardShortcut{
        .keys = {KEY_LEFTCTRL, KEY_A},
    });
    QSignalSpy activatedSpy(trigger.get(), &Trigger::activated);
    QSignalSpy endedSpy(trigger.get(), &Trigger::ended);

    KeyboardTriggerHandler handler;
    handler.addTrigger(std::move(trigger));

    QVERIFY(!handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_LEFTCTRL, true)));
    QVERIFY(handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_A, true)));
    handler.updateTriggers(TriggerType::KeyboardShortcut);
    QCOMPARE(activatedSpy.count(), 1);

    QVERIFY(handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_A, false)));
    QCOMPARE(endedSpy.count(), 1);

    QVERIFY(!handler.handleEvent(KeyboardKeyEvent(m_device.get(), KEY_LEFTCTRL, false)));
}

}

QTEST_MAIN(libinputactions::TestKeyboardTriggerHandler)
#include "TestKeyboardTriggerHandler.moc"