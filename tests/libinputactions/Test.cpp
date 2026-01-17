#include "Test.h"
#include <libinputactions/InputActionsMain.h>
#include <libinputactions/config/GlobalConfig.h>
#include <libinputactions/interfaces/ConfigProvider.h>

namespace InputActions
{

void Test::initMain()
{
    auto *inputActions = new InputActionsMain;
    g_configProvider = std::make_shared<ConfigProvider>(); // don't watch config
    inputActions->setMissingImplementations();
    g_globalConfig->setSendNotificationOnError(false);
    inputActions->initialize();
}

}

#include "Test.moc"