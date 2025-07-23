find_package(PkgConfig REQUIRED)
find_package(yaml-cpp REQUIRED)

pkg_search_module(LIBEVDEV REQUIRED libevdev)

set(libinputactions_SRCS
    libinputactions/actions/Action.cpp
    libinputactions/actions/ActionExecutor.cpp
    libinputactions/actions/ActionGroup.cpp
    libinputactions/actions/CommandAction.cpp
    libinputactions/actions/InputAction.cpp
    libinputactions/actions/PlasmaGlobalShortcutAction.cpp
    libinputactions/actions/TriggerAction.cpp
    libinputactions/conditions/CallbackCondition.cpp
    libinputactions/conditions/Condition.cpp
    libinputactions/conditions/ConditionGroup.cpp
    libinputactions/conditions/VariableCondition.cpp
    libinputactions/handlers/KeyboardTriggerHandler.cpp
    libinputactions/handlers/MotionTriggerHandler.cpp
    libinputactions/handlers/MouseTriggerHandler.cpp
    libinputactions/handlers/MultiTouchMotionTriggerHandler.cpp
    libinputactions/handlers/TouchpadTriggerHandler.cpp
    libinputactions/handlers/TriggerHandler.cpp
    libinputactions/input/backends/InputBackend.cpp
    libinputactions/input/backends/LibevdevComplementaryInputBackend.cpp
    libinputactions/input/backends/LibinputCompositorInputBackend.cpp
    libinputactions/input/InputDevice.cpp
    libinputactions/input/events.cpp
    libinputactions/input/InputEventHandler.cpp
    libinputactions/input/Keyboard.cpp
    libinputactions/interfaces/CursorShapeProvider.h
    libinputactions/interfaces/InputEmitter.h
    libinputactions/interfaces/OnScreenMessageManager.h
    libinputactions/interfaces/PointerPositionGetter.h
    libinputactions/interfaces/PointerPositionSetter.h
    libinputactions/interfaces/SessionLock.h
    libinputactions/interfaces/Window.h
    libinputactions/interfaces/WindowProvider.cpp
    libinputactions/triggers/DirectionalMotionTrigger.cpp
    libinputactions/triggers/KeyboardShortcutTrigger.cpp
    libinputactions/triggers/MotionTrigger.cpp
    libinputactions/triggers/PressTrigger.cpp
    libinputactions/triggers/StrokeTrigger.cpp
    libinputactions/triggers/Trigger.cpp
    libinputactions/triggers/WheelTrigger.cpp
    libinputactions/variables/LocalVariable.cpp
    libinputactions/variables/RemoteVariable.cpp
    libinputactions/variables/VariableManager.cpp
    libinputactions/variables/VariableOperations.cpp
    libinputactions/variables/Variable.cpp
    libinputactions/variables/VariableWrapper.h
    libinputactions/Config.cpp
    libinputactions/DBusInterface.cpp
    libinputactions/Expression.cpp
    libinputactions/globals.cpp
    libinputactions/InputActions.cpp
    libinputactions/Range.cpp
    libinputactions/Value.cpp
    libinputactions/yaml_convert.h
)

add_library(libinputactions STATIC ${libinputactions_SRCS})
target_link_libraries(libinputactions PUBLIC
    Qt6::Core
    Qt6::DBus
    ${LIBEVDEV_LIBRARIES}
)
target_compile_definitions(libinputactions PUBLIC TEST_VIRTUAL=$<IF:$<BOOL:${BUILD_TESTS}>,virtual,>)
target_include_directories(libinputactions PRIVATE libinputactions libinputactions/. ${LIBEVDEV_INCLUDE_DIRS})
set_target_properties(libinputactions PROPERTIES PREFIX "")

target_link_libraries(libinputactions PRIVATE yaml-cpp)
target_compile_options(libinputactions PUBLIC -fexceptions -fPIC)

if (INPUTACTIONS_BUILD_HYPRLAND)
    add_subdirectory(hyprland)
endif()
if (INPUTACTIONS_BUILD_KWIN)
    add_subdirectory(kwin)
endif()