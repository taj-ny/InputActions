#include "InputActions.h"
#include "actions/ActionExecutor.h"
#include "config/Config.h"
#include "input/Keyboard.h"
#include "input/backends/InputBackend.h"
#include "interfaces/CursorShapeProvider.h"
#include "interfaces/InputEmitter.h"
#include "interfaces/NotificationManager.h"
#include "interfaces/OnScreenMessageManager.h"
#include "interfaces/PointerPositionGetter.h"
#include "interfaces/PointerPositionSetter.h"
#include "interfaces/SessionLock.h"
#include "interfaces/WindowProvider.h"
#include "variables/VariableManager.h"
#include <QAbstractEventDispatcher>
#include <QCoreApplication>

namespace libinputactions
{

InputActions::InputActions(std::unique_ptr<InputBackend> inputBackend)
{
    g_inputActions = this;

    g_cursorShapeProvider = std::make_shared<CursorShapeProvider>();
    g_inputEmitter = std::make_shared<InputEmitter>();
    g_notificationManager = std::make_shared<NotificationManager>();
    g_onScreenMessageManager = std::make_shared<OnScreenMessageManager>();
    g_pointerPositionGetter = std::make_shared<PointerPositionGetter>();
    g_pointerPositionSetter = std::make_shared<PointerPositionSetter>();
    g_sessionLock = std::make_shared<SessionLock>();
    g_windowProvider = std::make_shared<WindowProvider>();

    g_config = std::make_unique<Config>();
    g_actionExecutor = std::make_unique<ActionExecutor>();
    g_inputBackend = std::move(inputBackend);
    g_keyboard = std::make_unique<Keyboard>();
    g_variableManager = std::make_unique<VariableManager>();
}

InputActions::~InputActions()
{
    // Release as many resources as possible when the compositor plugin is disabled (KWin doesn't unload plugins from the address space)
    g_cursorShapeProvider.reset();
    g_inputEmitter.reset();
    g_notificationManager.reset();
    g_onScreenMessageManager.reset();
    g_pointerPositionGetter.reset();
    g_pointerPositionSetter.reset();
    g_sessionLock.reset();
    g_windowProvider.reset();

    g_config.reset();
    g_actionExecutor.reset();
    g_inputBackend.reset();
    g_keyboard.reset();
    g_variableManager.reset();
}

void InputActions::runOnMainThread(std::function<void()> &&function, bool block)
{
    auto *mainThread = QCoreApplication::instance()->thread();
    if (QThread::currentThread() == mainThread) { // QThread::isMainThread requires Qt 6.8
        function();
    } else {
        QMetaObject::invokeMethod(QAbstractEventDispatcher::instance(mainThread), function, block ? Qt::BlockingQueuedConnection : Qt::QueuedConnection);
    }
}

}