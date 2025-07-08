cmake_minimum_required(VERSION 3.10)
project(mini-sql)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    src/frame_manager/disk_manager/disk_manager.cpp
)
add_library(mini-sql ${SOURCES})

target_include_directories(mini-sql PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(tests)