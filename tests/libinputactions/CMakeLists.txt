include_directories(.)

macro(libinputactions_add_test TARGET_NAME)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    qt_add_executable(${TARGET_NAME} ${TEST_SOURCES})
    add_test(NAME "${TARGET_NAME}" COMMAND ${TARGET_NAME})
    target_link_libraries(${TARGET_NAME} PRIVATE
        libinputactions
        GTest::gmock_main
        Qt::Core
        Qt::Test
    )
endmacro()

libinputactions_add_test(action SOURCES actions/TestAction.cpp)
libinputactions_add_test(actioninterval SOURCES actions/TestActionInterval.cpp)
libinputactions_add_test(directionalmotiontrigger SOURCES triggers/TestDirectionalMotionTrigger.cpp)
libinputactions_add_test(motiontrigger SOURCES triggers/TestMotionTrigger.cpp)
libinputactions_add_test(range SOURCES TestRange.cpp)
libinputactions_add_test(trigger SOURCES triggers/TestTrigger.cpp)
libinputactions_add_test(keyboardtriggerhandler SOURCES handlers/TestKeyboardTriggerHandler.cpp)
libinputactions_add_test(triggeraction SOURCES actions/TestTriggerAction.cpp)
libinputactions_add_test(triggerhandler SOURCES handlers/TestTriggerHandler.cpp)
libinputactions_add_test(touchpadtriggerhandler SOURCES handlers/TestTouchpadTriggerHandler.cpp)
libinputactions_add_test(value SOURCES TestValue.cpp)