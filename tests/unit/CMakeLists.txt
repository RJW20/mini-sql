add_library(test_utils STATIC utils.cpp)
target_include_directories(test_utils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/src
)
target_link_libraries(test_utils PUBLIC minisql)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/unit_test_paths.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/unit_test_paths.hpp
    @ONLY
)

file(GLOB_RECURSE TEST_SOURCES "test_*.cpp")

foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(${test_name} PRIVATE test_utils)
endforeach()