cmake_minimum_required(VERSION 3.15)
project(mini-sql VERSION 1.0 LANGUAGES CXX)

# Setup installation directories
include(GNUInstallDirs)

# -----------------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------------
option(BUILD_SHARED_LIBS
    "Build shared libraries (.dll/.so/.dylib) instead of static (.a/.lib)" OFF
)
option(BUILD_TESTING "Build tests" OFF)

# -----------------------------------------------------------------------------
# Library target
# -----------------------------------------------------------------------------
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
    src/row_set/row_iterator.cpp
    src/row_set/row_set.cpp
    src/engine/engine.cpp
    src/connection.cpp
)
add_library(minisql ${SOURCES})
add_library(minisql::minisql ALIAS minisql)

target_include_directories(minisql
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(BUILD_SHARED_LIBS)
    # Define MINISQL_EXPORTS for shared builds
    target_compile_definitions(minisql PRIVATE MINISQL_EXPORTS)
else()
    # Define MINISQL_STATIC for static builds and consumer
    target_compile_definitions(minisql
        PRIVATE MINISQL_STATIC
        INTERFACE MINISQL_STATIC
    )
endif()

# -----------------------------------------------------------------------------
# Optional tests
# -----------------------------------------------------------------------------
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Installation
# -----------------------------------------------------------------------------
# Install library and generate exported targets
install(
    TARGETS minisql
    EXPORT mini-sqlTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}     # DLLs on Windows
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}     # static lib or import lib
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}     # .so/.dylib
)

# Install headers
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets for find_package
install(
    EXPORT mini-sqlTargets
    NAMESPACE minisql::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mini-sql
)

# Generate config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mini-sqlConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
    "${CMAKE_CURRENT_LIST_DIR}/mini-sqlConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mini-sqlConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mini-sql
)

# Install config files
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/mini-sqlConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/mini-sqlConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mini-sql
)