
project(lal_hijack_c)

include_directories(
    ${PROJECT_SOURCE_DIR}/src/*.h
)

file(GLOB LAL_HIJACK_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/src/*.c
)

add_compile_definitions(USE_HIJACK)
add_library(${PROJECT_NAME} STATIC ${LAL_HIJACK_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    dl
)

include(../cmake/function.cmake)

add_subdirectory(${PROJECT_SOURCE_DIR}/examples)