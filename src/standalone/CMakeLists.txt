pkg_search_module(LIBINPUT REQUIRED libinput)
pkg_search_module(LIBUDEV REQUIRED libudev)

set(inputactions_SOURCES
    input/LibinputInputBackend.cpp
    main.cpp
)

add_executable(inputactions_standalone ${inputactions_SOURCES})
target_compile_options(inputactions_standalone PUBLIC -fexceptions)
target_link_libraries(inputactions_standalone PRIVATE
    libinputactions
    ${LIBINPUT_LIBRARIES}
    ${LIBUDEV_LIBRARIES}
)
install(TARGETS inputactions_standalone)