#include "TestInputTriggerHandler.h"
#include <QSignalSpy>
#include <libinputactions/handlers/InputTriggerHandler.h>
#include <libinputactions/input/events.h>
#include <libinputactions/triggers/Trigger.h>
#include <linux/input-event-codes.h>

namespace InputActions
{

void TestInputTriggerHandler::keyboardKey()
{
    InputDevice device(InputDeviceType::Keyboard);
    InputTriggerHandler handler;
    handler.setDevice(&device);
    QSignalSpy spy(&handler, &TriggerHandler::endingTriggers);
    handler.addTrigger(std::make_unique<Trigger>(TriggerType::Press));

    handler.activateTriggers(TriggerType::Press);
    handler.handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, true));
    QCOMPARE(spy.count(), 0);

    handler.handleEvent(KeyboardKeyEvent(&device, KEY_LEFTCTRL, false));
    QCOMPARE(spy.count(), 1);
}

}

QTEST_MAIN(InputActions::TestInputTriggerHandler)
#include "TestInputTriggerHandler.moc"