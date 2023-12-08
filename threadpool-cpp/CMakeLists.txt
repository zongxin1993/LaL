project(lal_threadpool_cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/examples/lal_threadpool_example.cpp)

target_link_libraries(${PROJECT_NAME}
        pthread
)

message(">>>> Added Executable: ${PROJECT_NAME} !")
