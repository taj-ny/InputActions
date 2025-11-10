#include "Test.h"
#include <libinputactions/InputActions.h>
#include <libinputactions/config/Config.h>
#include <libinputactions/interfaces/ConfigProvider.h>

namespace InputActions
{

void Test::initMain()
{
    auto *inputActions = new InputActions;
    g_configProvider = std::make_shared<ConfigProvider>(); // don't watch config
    inputActions->setMissingImplementations();
    g_config->setSendNotificationOnError(false);
    inputActions->initialize();
}

}

#include "Test.moc"