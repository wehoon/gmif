
file(GLOB_RECURSE _test_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB_RECURSE _gmif_srcs ${CMAKE_SOURCE_DIR}/src/*.cpp)
add_executable(gmif_unittest
        ${_test_srcs}
        ${_gmif_srcs})
unset(_test_srcs)
unset(_gmif_srcs)

find_package(GTest)
find_package(Threads)

exec_program(geos-config
        ARGS --ldflags
        OUTPUT_VARIABLE GEOS_LDFLAGS)

include_directories(${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GEOS_LDFLAGS}")

target_link_libraries(gmif_unittest
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        geos)

enable_testing()
add_test(NAME gmif_unittest
        COMMAND gmif_unittest
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

