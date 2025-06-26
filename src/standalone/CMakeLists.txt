set(inputactions_SOURCES
    main.cpp
)

add_executable(inputactions_standalone ${inputactions_SOURCES})
target_compile_options(inputactions_standalone PUBLIC -fexceptions)
target_link_libraries(inputactions_standalone PRIVATE
    libinputactions
)
install(TARGETS inputactions_standalone)