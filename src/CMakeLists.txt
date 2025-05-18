if (BUILD_KWIN_EFFECT)
    add_subdirectory(kwin)
endif()

find_package(PkgConfig REQUIRED)
find_package(yaml-cpp REQUIRED)

pkg_search_module(LIBEVDEV REQUIRED libevdev)

set(libinputactions_SRCS
    libinputactions/actions/action.cpp
    libinputactions/actions/actiongroup.cpp
    libinputactions/actions/command.cpp
    libinputactions/actions/plasmaglobalshortcut.cpp
    libinputactions/actions/input.cpp
    libinputactions/actions/oneactiongroup.cpp
    libinputactions/conditions/callbackcondition.cpp
    libinputactions/conditions/condition.cpp
    libinputactions/conditions/conditiongroup.cpp
    libinputactions/conditions/variable.cpp
    libinputactions/handlers/motion.cpp
    libinputactions/handlers/mouse.cpp
    libinputactions/handlers/multitouchmotion.cpp
    libinputactions/handlers/touchpad.cpp
    libinputactions/handlers/trigger.cpp
    libinputactions/input/backends/libevdev.cpp
    libinputactions/input/backend.cpp
    libinputactions/input/emitter.cpp
    libinputactions/input/events.cpp
    libinputactions/input/handler.cpp
    libinputactions/input/keyboard.cpp
    libinputactions/input/pointer.cpp
    libinputactions/triggers/directionalmotion.cpp
    libinputactions/triggers/motion.cpp
    libinputactions/triggers/press.cpp
    libinputactions/triggers/stroke.cpp
    libinputactions/triggers/trigger.cpp
    libinputactions/triggers/wheel.cpp
    libinputactions/variables/manager.cpp
    libinputactions/variables/operations.cpp
    libinputactions/variables/variable.cpp
    libinputactions/expression.cpp
    libinputactions/globals.h
    libinputactions/range.cpp
    libinputactions/value.cpp
    libinputactions/window.cpp
)

add_library(libinputactions STATIC ${libinputactions_SRCS})
target_link_libraries(libinputactions PRIVATE
    Qt6::Core
    Qt6::DBus
    ${LIBEVDEV_LIBRARIES}
)
target_compile_definitions(libinputactions PUBLIC TEST_VIRTUAL=$<IF:$<BOOL:${BUILD_TESTS}>,virtual,>)
target_include_directories(libinputactions PRIVATE libinputactions libinputactions/. ${LIBEVDEV_INCLUDE_DIRS})
set_target_properties(libinputactions PROPERTIES PREFIX "")

target_link_libraries(libinputactions PRIVATE yaml-cpp)
target_compile_options(libinputactions PUBLIC -fexceptions -fPIC)