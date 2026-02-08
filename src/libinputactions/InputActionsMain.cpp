#include "InputActionsMain.h"
#include "actions/ActionExecutor.h"
#include "config/ConfigLoader.h"
#include "config/GlobalConfig.h"
#include "input/StrokeRecorder.h"
#include "input/backends/InputBackend.h"
#include "interfaces/ConfigProvider.h"
#include "interfaces/CursorShapeProvider.h"
#include "interfaces/NotificationManager.h"
#include "interfaces/OnScreenMessageManager.h"
#include "interfaces/PointerPositionGetter.h"
#include "interfaces/PointerPositionSetter.h"
#include "interfaces/ProcessRunner.h"
#include "interfaces/SessionLock.h"
#include "interfaces/Window.h"
#include "interfaces/WindowProvider.h"
#include "interfaces/implementations/DBusNotificationManager.h"
#include "interfaces/implementations/DBusPlasmaGlobalShortcutInvoker.h"
#include "interfaces/implementations/FileConfigProvider.h"
#include "interfaces/implementations/ProcessRunnerImpl.h"
#include "variables/VariableManager.h"
#include <QFile>
#include <QStandardPaths>

namespace InputActions
{

InputActionsMain::InputActionsMain()
{
    g_inputActions = this;
}

InputActionsMain::~InputActionsMain()
{
    // Release as many resources as possible when the compositor plugin is disabled (KWin doesn't unload plugins from the address space)
    g_cursorShapeProvider.reset();
    g_notificationManager.reset();
    g_onScreenMessageManager.reset();
    g_pointerPositionGetter.reset();
    g_pointerPositionSetter.reset();
    g_processRunner.reset();
    g_sessionLock.reset();
    g_windowProvider.reset();

    g_actionExecutor.reset();
    g_configLoader.reset();
    g_globalConfig.reset();
    g_configProvider.reset();
    g_inputBackend.reset();
    g_strokeRecorder.reset();
    g_variableManager.reset();
}

void InputActionsMain::suspend()
{
    g_inputBackend->reset();
}

void InputActionsMain::initialize()
{
    connect(g_configProvider.get(), &ConfigProvider::configChanged, this, &InputActionsMain::onConfigChanged);
    registerGlobalVariables(g_variableManager.get());

    g_configLoader->loadEmpty(); // Initialize default values
}

void InputActionsMain::onConfigChanged(const QString &config)
{
    if (g_globalConfig->autoReload()) {
        g_configLoader->load({
            .config = config,
        });
    }
}

void InputActionsMain::setMissingImplementations()
{
    setMissingImplementation<ConfigProvider, FileConfigProvider>(g_configProvider);
    setMissingImplementation(g_cursorShapeProvider);
    setMissingImplementation<NotificationManager, DBusNotificationManager>(g_notificationManager);
    setMissingImplementation(g_onScreenMessageManager);
    setMissingImplementation(g_pointerPositionGetter);
    setMissingImplementation(g_pointerPositionSetter);
    setMissingImplementation<PlasmaGlobalShortcutInvoker, DBusPlasmaGlobalShortcutInvoker>(g_plasmaGlobalShortcutInvoker);
    setMissingImplementation<ProcessRunner, ProcessRunnerImpl>(g_processRunner);
    setMissingImplementation(g_sessionLock);
    setMissingImplementation(g_windowProvider);

    setMissingImplementation(g_actionExecutor);
    setMissingImplementation(g_configLoader);
    setMissingImplementation(g_globalConfig);
    setMissingImplementation(g_inputBackend);
    setMissingImplementation(g_strokeRecorder);
    setMissingImplementation(g_variableManager);
}

void InputActionsMain::registerGlobalVariables(VariableManager *variableManager, std::shared_ptr<PointerPositionGetter> pointerPositionGetter,
                                               std::shared_ptr<WindowProvider> windowProvider)
{
    if (!pointerPositionGetter) {
        pointerPositionGetter = g_pointerPositionGetter;
    }
    if (!windowProvider) {
        windowProvider = g_windowProvider;
    }

    variableManager->registerRemoteVariable<CursorShape>("cursor_shape", [](auto &value) {
        value = g_cursorShapeProvider->cursorShape();
    });
    variableManager->registerLocalVariable(BuiltinVariables::DeviceName);
    for (auto i = 1; i <= s_fingerVariableCount; i++) {
        variableManager->registerLocalVariable<QPointF>(QString("finger_%1_initial_position_percentage").arg(i));
        variableManager->registerLocalVariable<QPointF>(QString("finger_%1_position_percentage").arg(i));
        variableManager->registerLocalVariable<qreal>(QString("finger_%1_pressure").arg(i));
    }
    variableManager->registerLocalVariable(BuiltinVariables::Fingers);
    variableManager->registerRemoteVariable<Qt::KeyboardModifiers>(BuiltinVariables::KeyboardModifiers, [](auto &value) {
        value = g_inputBackend->keyboardModifiers();
    });
    variableManager->registerLocalVariable(BuiltinVariables::LastTriggerId);
    variableManager->registerLocalVariable(BuiltinVariables::LastTriggerTimestamp, true);
    variableManager->registerRemoteVariable<QPointF>("pointer_position_screen_percentage", [pointerPositionGetter](auto &value) {
        value = pointerPositionGetter->screenPointerPosition();
    });
    variableManager->registerRemoteVariable<QPointF>("pointer_position_window_percentage", [pointerPositionGetter, windowProvider](auto &value) {
        const auto window = windowProvider->windowUnderPointer();
        if (!window) {
            return;
        }

        const auto windowGeometry = window->geometry();
        const auto pointerPos = pointerPositionGetter->globalPointerPosition();
        if (!pointerPos || !windowGeometry) {
            return;
        }
        const auto translatedPosition = pointerPos.value() - windowGeometry->topLeft();
        value = QPointF(translatedPosition.x() / windowGeometry->width(), translatedPosition.y() / windowGeometry->height());
    });
    variableManager->registerLocalVariable(BuiltinVariables::ThumbInitialPositionPercentage);
    variableManager->registerLocalVariable(BuiltinVariables::ThumbPositionPercentage);
    variableManager->registerLocalVariable(BuiltinVariables::ThumbPresent);
    variableManager->registerRemoteVariable<qreal>("time_since_last_trigger", [variableManager](auto &value) {
        value = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count()
              - variableManager->getVariable(BuiltinVariables::LastTriggerTimestamp)->get().value_or(0);
    });
    variableManager->registerRemoteVariable<QString>("window_class", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->resourceClass();
        }
    });
    variableManager->registerRemoteVariable<bool>("window_fullscreen", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->fullscreen();
        }
    });
    variableManager->registerRemoteVariable<QString>("window_id", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->id();
        }
    });
    variableManager->registerRemoteVariable<bool>("window_maximized", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->maximized();
        }
    });
    variableManager->registerRemoteVariable<QString>("window_name", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->resourceName();
        }
    });
    variableManager->registerRemoteVariable<qreal>("window_pid", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->pid();
        }
    });
    variableManager->registerRemoteVariable<QString>("window_title", [windowProvider](auto &value) {
        if (const auto window = windowProvider->activeWindow()) {
            value = window->title();
        }
    });
    variableManager->registerRemoteVariable<QString>("window_under_pointer_class", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->resourceClass();
        }
    });
    variableManager->registerVariableAlias("window_under_class", "window_under_pointer_class");
    variableManager->registerRemoteVariable<bool>("window_under_pointer_fullscreen", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->fullscreen();
        }
    });
    variableManager->registerVariableAlias("window_under_fullscreen", "window_under_pointer_fullscreen");
    variableManager->registerRemoteVariable<QString>("window_under_pointer_id", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->id();
        }
    });
    variableManager->registerVariableAlias("window_under_id", "window_under_pointer_id");
    variableManager->registerRemoteVariable<bool>("window_under_pointer_maximized", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->maximized();
        }
    });
    variableManager->registerVariableAlias("window_under_maximized", "window_under_pointer_maximized");
    variableManager->registerRemoteVariable<QString>("window_under_pointer_name", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->resourceName();
        }
    });
    variableManager->registerVariableAlias("window_under_name", "window_under_pointer_name");
    variableManager->registerRemoteVariable<qreal>("window_under_pointer_pid", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->pid();
        }
    });
    variableManager->registerVariableAlias("window_under_pid", "window_under_pointer_pid");
    variableManager->registerRemoteVariable<QString>("window_under_pointer_title", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderPointer()) {
            value = window->title();
        }
    });
    variableManager->registerVariableAlias("window_under_title", "window_under_pointer_title");
    variableManager->registerRemoteVariable<QString>("window_under_fingers_class", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->resourceClass();
        }
    });
    variableManager->registerRemoteVariable<bool>("window_under_fingers_fullscreen", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->fullscreen();
        }
    });
    variableManager->registerRemoteVariable<QString>("window_under_fingers_id", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->id();
        }
    });
    variableManager->registerRemoteVariable<bool>("window_under_fingers_maximized", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->maximized();
        }
    });
    variableManager->registerRemoteVariable<QString>("window_under_fingers_name", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->resourceName();
        }
    });
    variableManager->registerRemoteVariable<qreal>("window_under_fingers_pid", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->pid();
        }
    });
    variableManager->registerRemoteVariable<QString>("window_under_fingers_title", [windowProvider](auto &value) {
        if (const auto window = windowProvider->windowUnderFingers()) {
            value = window->title();
        }
    });
}

}