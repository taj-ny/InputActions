find_package(PkgConfig REQUIRED)
find_package(yaml-cpp REQUIRED)

pkg_search_module(LIBEVDEV REQUIRED libevdev)

set(libinputactions_SRCS
    libinputactions/actions/CommandTriggerAction.cpp
    libinputactions/actions/InputTriggerAction.cpp
    libinputactions/actions/OneTriggerActionGroup.cpp
    libinputactions/actions/PlasmaGlobalShortcutTriggerAction.cpp
    libinputactions/actions/TriggerAction.cpp
    libinputactions/actions/TriggerActionGroup.cpp
    libinputactions/conditions/CallbackCondition.cpp
    libinputactions/conditions/Condition.cpp
    libinputactions/conditions/ConditionGroup.cpp
    libinputactions/conditions/VariableCondition.cpp
    libinputactions/handlers/MotionTriggerHandler.cpp
    libinputactions/handlers/MouseTriggerHandler.cpp
    libinputactions/handlers/MultiTouchMotionTriggerHandler.cpp
    libinputactions/handlers/TouchpadTriggerHandler.cpp
    libinputactions/handlers/TriggerHandler.cpp
    libinputactions/input/backends/InputBackend.cpp
    libinputactions/input/backends/LibevdevComplementaryInputBackend.cpp
    libinputactions/input/InputDevice.cpp
    libinputactions/input/InputEmitter.cpp
    libinputactions/input/events.cpp
    libinputactions/input/InputEventHandler.cpp
    libinputactions/input/Keyboard.cpp
    libinputactions/interfaces/CursorShapeProvider.cpp
    libinputactions/interfaces/InputEmitter.cpp
    libinputactions/interfaces/PointerPositionGetter.cpp
    libinputactions/interfaces/PointerPositionSetter.cpp
    libinputactions/interfaces/SessionLock.cpp
    libinputactions/interfaces/Window.cpp
    libinputactions/interfaces/WindowProvider.cpp
    libinputactions/triggers/DirectionalMotionTrigger.cpp
    libinputactions/triggers/MotionTrigger.cpp
    libinputactions/triggers/PressTrigger.cpp
    libinputactions/triggers/StrokeTrigger.cpp
    libinputactions/triggers/Trigger.cpp
    libinputactions/triggers/WheelTrigger.cpp
    libinputactions/variables/VariableManager.cpp
    libinputactions/variables/VariableOperations.cpp
    libinputactions/variables/Variable.cpp
    libinputactions/Config.cpp
    libinputactions/Expression.cpp
    libinputactions/globals.cpp
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

if (BUILD_HYPRLAND_PLUGIN)
    add_subdirectory(hyprland)
endif()
if (BUILD_KWIN_EFFECT)
    add_subdirectory(kwin)
endif()