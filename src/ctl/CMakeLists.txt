find_package(CLI11 CONFIG REQUIRED)

add_executable(inputactionsctl
    main.cpp
)
target_link_libraries(inputactionsctl PRIVATE
    CLI11::CLI11
    Qt6::DBus
)

install(TARGETS inputactionsctl)