project(lal_cgraph_perf)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/CGraph-perf/CGraph/src)

include(${CMAKE_SOURCE_DIR}/CGraph-perf/CGraph/cmake/CGraph-env-include.cmake)

include_directories(
        ${CMAKE_SOURCE_DIR}/CGraph-perf/taskflow
)

add_executable(lal_tf_perf src/tf-perf.cpp)
add_executable(lal_cg_perf
        $<TARGET_OBJECTS:CGraph>
        src/cg-perf.cpp)
add_executable(lal_cg_perf_rt
        $<TARGET_OBJECTS:CGraph>
        src/cg-perf-rt.cpp)

add_executable(lal_lttng_test src/mutex_test.cpp)


