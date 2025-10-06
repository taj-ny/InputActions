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

libinputactions_add_test(Action SOURCES actions/TestAction.cpp)
libinputactions_add_test(ActionExecutor SOURCES actions/TestActionExecutor.cpp)
libinputactions_add_test(ActionInterval SOURCES actions/TestActionInterval.cpp)
libinputactions_add_test(DirectionalMotionTrigger SOURCES triggers/TestDirectionalMotionTrigger.cpp)
libinputactions_add_test(InputTriggerHandler SOURCES handlers/TestInputTriggerHandler.cpp)
libinputactions_add_test(KeyboardTriggerHandler SOURCES handlers/TestKeyboardTriggerHandler.cpp)
libinputactions_add_test(MotionTrigger SOURCES triggers/TestMotionTrigger.cpp)
libinputactions_add_test(PointerTriggerHandler SOURCES handlers/TestPointerTriggerHandler.cpp)
libinputactions_add_test(Range SOURCES TestRange.cpp)
libinputactions_add_test(TouchpadTriggerHandler SOURCES handlers/TestTouchpadTriggerHandler.cpp)
libinputactions_add_test(Trigger SOURCES triggers/TestTrigger.cpp)
libinputactions_add_test(TriggerAction SOURCES actions/TestTriggerAction.cpp)
libinputactions_add_test(TriggerHandler SOURCES handlers/TestTriggerHandler.cpp)
libinputactions_add_test(Value SOURCES TestValue.cpp)