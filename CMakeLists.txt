cmake_minimum_required(VERSION 3.10)
project(mini-sql)

find_package(Python3 REQUIRED)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    src/frame_manager/disk_manager/disk_manager.cpp
    src/frame_manager/cache/frame_view.cpp
    src/frame_manager/cache/cache.cpp
    src/frame_manager/free_list/free_list.cpp
    src/bplus_tree/node.cpp
    src/bplus_tree/internal_node.cpp
    src/bplus_tree/leaf_node.cpp
    src/bplus_tree/bplus_tree.cpp
    src/row/row.cpp
    src/cursor.cpp
    src/planner/compiler.cpp
    src/planner/planner.cpp
    src/validator/validator.cpp
    src/parser/parser.cpp
    src/database.cpp
    src/engine/database_handle.cpp
    src/engine/engine.cpp
)
add_library(minisql STATIC ${SOURCES})

target_include_directories(minisql PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(tests)