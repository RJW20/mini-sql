cmake_minimum_required(VERSION 3.10)
project(mini-sql)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    src/frame_manager/disk_manager/disk_manager.cpp
    src/frame_manager/cache/frame_view.cpp
    src/frame_manager/cache/cache.cpp
    src/frame_manager/free_list/free_list.cpp
)
add_library(mini-sql ${SOURCES})

target_include_directories(mini-sql PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(tests)