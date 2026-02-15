if (INPUTACTIONS_BUILD_HYPRLAND OR INPUTACTIONS_BUILD_KWIN OR INPUTACTIONS_BUILD_STANDALONE OR BUILD_TESTS)
    find_package(PkgConfig REQUIRED)
    find_package(yaml-cpp REQUIRED)

    pkg_search_module(LIBEVDEV REQUIRED libevdev)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/libevdev-cpp ${CMAKE_CURRENT_BINARY_DIR}/../lib/libevdev-cpp)

    set(libinputactions_SRCS
        libinputactions/actions/Action.cpp
        libinputactions/actions/ActionExecutor.cpp
        libinputactions/actions/ActionGroup.cpp
        libinputactions/actions/CommandAction.cpp
        libinputactions/actions/CustomAction.cpp
        libinputactions/actions/InputAction.cpp
        libinputactions/actions/PlasmaGlobalShortcutAction.cpp
        libinputactions/actions/SleepAction.cpp
        libinputactions/actions/TriggerAction.cpp
        libinputactions/config/ConfigLoader.cpp
        libinputactions/config/GlobalConfig.cpp
        libinputactions/config/yaml.h
        libinputactions/conditions/Condition.cpp
        libinputactions/conditions/ConditionGroup.cpp
        libinputactions/conditions/CustomCondition.cpp
        libinputactions/conditions/LazyCondition.cpp
        libinputactions/conditions/VariableCondition.cpp
        libinputactions/dbus/DBusInterfaceBase.cpp
        libinputactions/dbus/IntegratedDBusInterface.cpp
        libinputactions/handlers/InputTriggerHandler.cpp
        libinputactions/handlers/KeyboardTriggerHandler.cpp
        libinputactions/handlers/MotionTriggerHandler.cpp
        libinputactions/handlers/MouseTriggerHandler.cpp
        libinputactions/handlers/MultiTouchMotionTriggerHandler.cpp
        libinputactions/handlers/PointerTriggerHandler.cpp
        libinputactions/handlers/TouchpadTriggerHandler.cpp
        libinputactions/handlers/TouchscreenTriggerHandler.cpp
        libinputactions/handlers/TriggerHandler.cpp
        libinputactions/input/backends/InputBackend.cpp
        libinputactions/input/backends/LibevdevComplementaryInputBackend.cpp
        libinputactions/input/backends/LibinputInputBackend.cpp
        libinputactions/input/devices/InputDevice.cpp
        libinputactions/input/devices/InputDeviceProperties.cpp
        libinputactions/input/devices/InputDeviceRule.cpp
        libinputactions/input/devices/InputDeviceState.cpp
        libinputactions/input/devices/VirtualKeyboard.cpp
        libinputactions/input/devices/VirtualMouse.cpp
        libinputactions/input/Delta.cpp
        libinputactions/input/InputEventHandler.cpp
        libinputactions/input/KeyboardKey.cpp
        libinputactions/input/MouseButton.cpp
        libinputactions/input/StrokeRecorder.cpp
        libinputactions/input/events.cpp
        libinputactions/interfaces/implementations/DBusNotificationManager.cpp
        libinputactions/interfaces/implementations/DBusPlasmaGlobalShortcutInvoker.cpp
        libinputactions/interfaces/implementations/FileConfigProvider.cpp
        libinputactions/interfaces/implementations/ProcessRunnerImpl.cpp
        libinputactions/interfaces/ConfigProvider.cpp
        libinputactions/interfaces/CursorShapeProvider.h
        libinputactions/interfaces/NotificationManager.h
        libinputactions/interfaces/OnScreenMessageManager.h
        libinputactions/interfaces/PlasmaGlobalShortcutInvoker.h
        libinputactions/interfaces/PointerPositionGetter.h
        libinputactions/interfaces/PointerPositionSetter.h
        libinputactions/interfaces/ProcessRunner.cpp
        libinputactions/interfaces/SessionLock.h
        libinputactions/interfaces/Window.h
        libinputactions/interfaces/WindowProvider.cpp
        libinputactions/triggers/DirectionalMotionTrigger.cpp
        libinputactions/triggers/HoverTrigger.cpp
        libinputactions/triggers/KeyboardShortcutTrigger.cpp
        libinputactions/triggers/MotionTrigger.cpp
        libinputactions/triggers/PressTrigger.cpp
        libinputactions/triggers/StrokeTrigger.cpp
        libinputactions/triggers/Trigger.cpp
        libinputactions/triggers/WheelTrigger.cpp
        libinputactions/utils/QVariantUtils.cpp
        libinputactions/utils/ThreadUtils.cpp
        libinputactions/utils/SessionUtils.cpp
        libinputactions/utils/StringUtils.cpp
        libinputactions/variables/LocalVariable.cpp
        libinputactions/variables/RemoteVariable.cpp
        libinputactions/variables/VariableManager.cpp
        libinputactions/variables/VariableOperations.cpp
        libinputactions/variables/Variable.cpp
        libinputactions/variables/VariableWrapper.h
        libinputactions/globals.cpp
        libinputactions/InputActionsMain.cpp
        libinputactions/Range.cpp
        libinputactions/Value.cpp
    )
    if (INPUTACTIONS_BUILD_IPC)
        list(APPEND libinputactions_SRCS
            libinputactions/ipc/JsonSerializer.cpp
            libinputactions/ipc/messages.cpp
            libinputactions/ipc/MessageHandler.cpp
            libinputactions/ipc/MessageSocketConnection.cpp)
    endif()

    add_library(libinputactions STATIC ${libinputactions_SRCS})
    target_link_libraries(libinputactions PUBLIC
        libevdev-cpp
        Qt6::Core
        Qt6::DBus
        ${LIBEVDEV_LIBRARIES}
    )
    if (INPUTACTIONS_BUILD_IPC)
        target_link_libraries(libinputactions PUBLIC
            Qt6::Network
        )
    endif()
    target_compile_definitions(libinputactions PUBLIC TEST_VIRTUAL=$<IF:$<BOOL:${BUILD_TESTS}>,virtual,>)
    target_include_directories(libinputactions PRIVATE libinputactions libinputactions/. ${LIBEVDEV_INCLUDE_DIRS})
    set_target_properties(libinputactions PROPERTIES PREFIX "")

    target_link_libraries(libinputactions PRIVATE yaml-cpp)
    target_compile_options(libinputactions PUBLIC -fexceptions -fPIC)
endif()

if (INPUTACTIONS_BUILD_CTL)
    add_subdirectory(ctl)
endif()
if (INPUTACTIONS_BUILD_HYPRLAND)
    add_subdirectory(hyprland)
endif()
if (INPUTACTIONS_BUILD_KWIN)
    add_subdirectory(kwin)
endif()
if (INPUTACTIONS_BUILD_STANDALONE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../lib/libinput-cpp ${CMAKE_CURRENT_BINARY_DIR}/../lib/libinput-cpp)
    add_subdirectory(standalone)
endif()