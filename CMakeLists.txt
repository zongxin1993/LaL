cmake_minimum_required(VERSION 3.15)

# set project name from current directory
get_filename_component(BASENAME ${CMAKE_SOURCE_DIR} NAME)

set(LAL_VERSION_MAJOR 0)
set(LAL_VERSION_MINOR 1)
set(LAL_VERSION_STRING ${LAL_VERSION_MAJOR}.${LAL_VERSION_MINOR})

project(lal)
include(GNUInstallDirs)

set(CMAKE_BUILD_TYPE "Debug")

set(LAL lal)
set(LAL_RUNTIME_ETC_DIR "${CMAKE_INSTALL_SYSCONFDIR}")
set(LAL_INSTALL_ETC_DIR "${CMAKE_INSTALL_SYSCONFDIR}")
set(LAL_INSTALL_INC_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
set(LAL_INSTALL_LIB_DIR "${CMAKE_INSTALL_LIBDIR}")
set(LAL_INSTALL_BIN_DIR "${CMAKE_INSTALL_BINDIR}")

include(cmake/gtest.cmake)

add_subdirectory(logger-cpp)
add_subdirectory(socket-cpp)
add_subdirectory(hijack-c)
add_subdirectory(threadpool-cpp)