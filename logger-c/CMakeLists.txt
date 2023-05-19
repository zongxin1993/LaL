
project(lal_logger_cpp)

set(CMAKE_CXX_STANDARD 14)

include_directories(
    ${PROJECT_SOURCE_DIR}/src/*.h
)

# add_library(${PROJECT_NAME} STATIC ${LAL_LOGGER_SOURCE_FILES})
include(../cmake/function.cmake)

add_subdirectory(${PROJECT_SOURCE_DIR}/examples)