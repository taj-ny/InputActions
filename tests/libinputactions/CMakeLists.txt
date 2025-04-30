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

libinputactions_add_test(action SOURCES actions/test_action.cpp)
libinputactions_add_test(actioninterval SOURCES actions/test_actioninterval.cpp)
libinputactions_add_test(conditiongroup SOURCES conditions/test_conditiongroup.cpp)
libinputactions_add_test(directionalmotiontrigger SOURCES triggers/test_directionalmotiontrigger.cpp)
libinputactions_add_test(motiontrigger SOURCES triggers/test_motiontrigger.cpp)
libinputactions_add_test(range SOURCES test_range.cpp)
libinputactions_add_test(trigger SOURCES triggers/test_trigger.cpp)
libinputactions_add_test(triggerhandler SOURCES handlers/test_triggerhandler.cpp)
