
project(lal_socket_cpp)

set(CMAKE_CXX_STANDARD 14)

file(GLOB LAL_SOCKET_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.c
)

include_directories(
    ${PROJECT_SOURCE_DIR}/src/*.hh
    ${PROJECT_SOURCE_DIR}/src/*.h
)

add_library(${PROJECT_NAME} STATIC ${LAL_SOCKET_SOURCE_FILES})

include(../cmake/function.cmake)

add_subdirectory(${PROJECT_SOURCE_DIR}/examples)

# add_subdirectory(${PROJECT_SOURCE_DIR}/tests)