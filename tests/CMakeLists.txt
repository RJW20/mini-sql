add_library(test_utils STATIC utils.cpp)
target_include_directories(test_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_utils PRIVATE mini-sql)

file(GLOB_RECURSE TEST_SOURCES "test_*.cpp")

foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE mini-sql test_utils)
endforeach()