include_directories(.)

macro(libinputactions_add_test TARGET_NAME)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    qt_add_executable(${TARGET_NAME} ${TEST_SOURCES} Test.cpp utils.cpp)
    add_test(NAME "${TARGET_NAME}" COMMAND ${TARGET_NAME})
    target_link_libraries(${TARGET_NAME} PRIVATE
        libinputactions
        GTest::gmock_main
        Qt::Core
        Qt::Test
    )
endmacro()

libinputactions_add_test(TestAction SOURCES actions/TestAction.cpp)
libinputactions_add_test(TestActionExecutor SOURCES actions/TestActionExecutor.cpp)
libinputactions_add_test(TestActionInterval SOURCES actions/TestActionInterval.cpp)
libinputactions_add_test(TestConditionGroup SOURCES conditions/TestConditionGroup.cpp)
libinputactions_add_test(TestDirectionalMotionTrigger SOURCES triggers/TestDirectionalMotionTrigger.cpp)
libinputactions_add_test(TestInputTriggerHandler SOURCES handlers/TestInputTriggerHandler.cpp)
libinputactions_add_test(TestKeyboardTriggerHandler SOURCES handlers/TestKeyboardTriggerHandler.cpp)
libinputactions_add_test(TestMotionTrigger SOURCES triggers/TestMotionTrigger.cpp)
libinputactions_add_test(TestPointerTriggerHandler SOURCES handlers/TestPointerTriggerHandler.cpp)
libinputactions_add_test(TestRange SOURCES TestRange.cpp)
libinputactions_add_test(TestTouchpadTriggerHandler SOURCES handlers/TestTouchpadTriggerHandler.cpp)
libinputactions_add_test(TestTrigger SOURCES triggers/TestTrigger.cpp)
libinputactions_add_test(TestTriggerAction SOURCES actions/TestTriggerAction.cpp)
libinputactions_add_test(TestTriggerHandler SOURCES handlers/TestTriggerHandler.cpp)
libinputactions_add_test(TestValue SOURCES TestValue.cpp)