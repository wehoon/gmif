cmake_minimum_required(VERSION 3.8)
project(gmif)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif (NOT CMAKE_CXX_STANDARD)

exec_program(geos-config
        ARGS --includes
        OUTPUT_VARIABLE GEOS_INCLUDE_PATH)

include_directories(${GEOS_INCLUDE_PATH})
add_definitions(-DUSE_UNSTABLE_GEOS_CPP_API)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCS)
add_library(gmif ${SRCS})

target_include_directories(gmif
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        )

if (BUILD_TEST)
    add_subdirectory(test)
endif ()