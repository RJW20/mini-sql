include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE TEST_SOURCES "test_*.cpp")

foreach(test_src ${TEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE mini-sql)
endforeach()