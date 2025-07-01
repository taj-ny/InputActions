cmake_minimum_required(VERSION 3.16.0)

project(inputactions)
set(PROJECT_VERSION "0.7.0")

set(QT_MIN_VERSION "6.6.0")
set(QT_MAJOR_VERSION 6)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# TODO Find a way to split this into multiple packages
set(CPACK_PACKAGE_NAME "inputactions")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_PACKAGE_FILE_NAME "inputactions")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Mouse and touchpad gestures for Hyprland, Plasma 6 Wayland")
include(CPack)

option(INPUTACTIONS_BUILD_HYPRLAND "Build the Hyprland plugin" OFF)
option(INPUTACTIONS_BUILD_KWIN "Build the KWin plugin" OFF)
option(BUILD_TESTS "Build tests" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -DQT_NO_DEBUG_OUTPUT")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(-DDEBUG)
endif()

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
)

add_compile_options(-Wno-unused-parameter) # This warning is stupid

add_compile_definitions(PROJECT_NAME="InputActions")
add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")

add_subdirectory(src)
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()