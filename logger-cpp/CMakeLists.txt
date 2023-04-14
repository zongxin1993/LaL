
project(lal_logger_cpp)

set(CMAKE_CXX_STANDARD 14)

file(GLOB LAL_LOGGER_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/src/*.hh
)

add_library(${PROJECT_NAME} STATIC ${LAL_LOGGER_SOURCE_FILES})

include(../cmake/function.cmake)

add_subdirectory(${PROJECT_SOURCE_DIR}/examples)
add_subdirectory(${PROJECT_SOURCE_DIR}/tests)