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
#include "interfaces/Window.h"
#include "interfaces/WindowProvider.h"
#include "variables/VariableManager.h"
#include <QAbstractEventDispatcher>
#include <QCoreApplication>

namespace InputActions
{

InputActions::InputActions()
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
    g_inputBackend = std::make_unique<InputBackend>();
    g_keyboard = std::make_unique<Keyboard>();
    g_variableManager = std::make_unique<VariableManager>();
    registerGlobalVariables();
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

void InputActions::registerGlobalVariables()
{
    g_variableManager->registerRemoteVariable<CursorShape>("cursor_shape", [](auto &value) {
        value = g_cursorShapeProvider->cursorShape();
    });
    g_variableManager->registerLocalVariable(BuiltinVariables::DeviceName);
    for (auto i = 1; i <= s_fingerVariableCount; i++) {
        g_variableManager->registerLocalVariable<QPointF>(QString("finger_%1_initial_position_percentage").arg(i));
        g_variableManager->registerLocalVariable<QPointF>(QString("finger_%1_position_percentage").arg(i));
        g_variableManager->registerLocalVariable<qreal>(QString("finger_%1_pressure").arg(i));
    }
    g_variableManager->registerLocalVariable(BuiltinVariables::Fingers);
    g_variableManager->registerRemoteVariable<Qt::KeyboardModifiers>(BuiltinVariables::KeyboardModifiers, [](auto &value) {
        value = g_keyboard->modifiers();
    });
    g_variableManager->registerLocalVariable(BuiltinVariables::LastTriggerId);
    g_variableManager->registerLocalVariable(BuiltinVariables::LastTriggerTimestamp, true);
    g_variableManager->registerRemoteVariable<QPointF>("pointer_position_screen_percentage", [](auto &value) {
        value = g_pointerPositionGetter->screenPointerPosition();
    });
    g_variableManager->registerRemoteVariable<QPointF>("pointer_position_window_percentage", [](auto &value) {
        const auto window = g_windowProvider->windowUnderPointer();
        if (!window) {
            return;
        }

        const auto windowGeometry = window->geometry();
        const auto pointerPos = g_pointerPositionGetter->globalPointerPosition();
        if (!pointerPos || !windowGeometry) {
            return;
        }
        const auto translatedPosition = pointerPos.value() - windowGeometry->topLeft();
        value = QPointF(translatedPosition.x() / windowGeometry->width(), translatedPosition.y() / windowGeometry->height());
    });
    g_variableManager->registerLocalVariable(BuiltinVariables::ThumbInitialPositionPercentage);
    g_variableManager->registerLocalVariable(BuiltinVariables::ThumbPositionPercentage);
    g_variableManager->registerLocalVariable(BuiltinVariables::ThumbPresent);
    g_variableManager->registerRemoteVariable<qreal>("time_since_last_trigger", [](auto &value) {
        value = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()
              - g_variableManager->getVariable(BuiltinVariables::LastTriggerTimestamp)->get().value_or(0);
    });
    g_variableManager->registerRemoteVariable<QString>("window_class", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->resourceClass();
        }
    });
    g_variableManager->registerRemoteVariable<bool>("window_fullscreen", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->fullscreen();
        }
    });
    g_variableManager->registerRemoteVariable<QString>("window_id", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->id();
        }
    });
    g_variableManager->registerRemoteVariable<bool>("window_maximized", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->maximized();
        }
    });
    g_variableManager->registerRemoteVariable<QString>("window_name", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->resourceName();
        }
    });
    g_variableManager->registerRemoteVariable<QString>("window_title", [](auto &value) {
        if (const auto window = g_windowProvider->activeWindow()) {
            value = window->title();
        }
    });
    g_variableManager->registerRemoteVariable<QString>("window_under_class", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->resourceClass();
        }
    });
    g_variableManager->registerRemoteVariable<bool>("window_under_fullscreen", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->fullscreen();
        }
    });
    g_variableManager->registerRemoteVariable<QString>("window_under_id", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->id();
        }
    });
    g_variableManager->registerRemoteVariable<bool>("window_under_maximized", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->maximized();
        }
    });
    g_variableManager->registerRemoteVariable<QString>("window_under_name", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->resourceName();
        }
    });
    g_variableManager->registerRemoteVariable<QString>("window_under_title", [](auto &value) {
        if (const auto window = g_windowProvider->windowUnderPointer()) {
            value = window->title();
        }
    });
}

}