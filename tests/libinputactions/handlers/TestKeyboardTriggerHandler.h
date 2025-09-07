#pragma once

#include <QTest>
#include <libinputactions/input/InputDevice.h>

namespace libinputactions
{

class TestKeyboardTriggerHandler : public QObject
{
    Q_OBJECT

private slots:
    void init();

    void shortcut_oneModifierKeyShortcut_triggerActivatedEndedAndEventsNotBlocked();
    void shortcut_oneNonModifierKeyShortcut_triggerActivatedEndedAndEventsBlocked();
    void shortcut_twoKeysWrongOrder_triggerNotActivatedAndEventsNotBlocked();
    void shortcut_twoKeysCorrectOrder_triggerActivatedAndNormalKeyBlockedAndModifierKeyNotBlocked();

private:
    std::unique_ptr<InputDevice> m_device;
};

}