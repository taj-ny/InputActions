#include "TestKeyboardTriggerHandler.h"
#include <QSignalSpy>
#include <libinputactions/handlers/KeyboardTriggerHandler.h>
#include <libinputactions/triggers/KeyboardShortcutTrigger.h>
#include <linux/input-event-codes.h>

namespace libinputactions
{

void TestKeyboardTriggerHandler::handleEvent_keyboardKey()
{
    auto handler = std::make_unique<KeyboardTriggerHandler>();
    QSignalSpy activatingSpy(handler.get(), &TriggerHandler::activatingTrigger);
    QSignalSpy endingSpy(handler.get(), &TriggerHandler::endingTriggers);
    handler->addTrigger(std::make_unique<KeyboardShortcutTrigger>(KeyboardShortcut{
        .keys = {KEY_LEFTCTRL, KEY_A}
    }));

    InputDevice device(InputDeviceType::Keyboard);
    QCOMPARE(handler->handleEvent(KeyboardKeyEvent(&device, KEY_A, true)), false);
    QCOMPARE(handler->handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, true)), false);
    QCOMPARE(activatingSpy.count(), 0);

    QCOMPARE(handler->handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, false)), false);
    QCOMPARE(handler->handleEvent(KeyboardKeyEvent(&device, KEY_A, false)), false);

    QCOMPARE(handler->handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, true)), false);
    QCOMPARE(handler->handleEvent(KeyboardKeyEvent(&device, KEY_A, true)), true);
    QCOMPARE(activatingSpy.count(), 1);

    QCOMPARE(handler->handleEvent(KeyboardKeyEvent(&device, KEY_A, false)), true);
    QCOMPARE(endingSpy.count(), 1);
    QCOMPARE(handler->handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, false)), false);

    QCOMPARE(activatingSpy.count(), 1);
    QCOMPARE(endingSpy.count(), 1);
}

}

QTEST_MAIN(libinputactions::TestKeyboardTriggerHandler)
#include "TestKeyboardTriggerHandler.moc"