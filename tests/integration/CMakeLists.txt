# Integration test executable
add_executable(runner runner.cpp)
target_link_libraries(runner PRIVATE mini-sql)

# Generator scripts
set(GEN_TEST_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/gen_tests.py)
set(GEN_STAMP_FILE ${CMAKE_CURRENT_BINARY_DIR}/regen.stamp)

# Gather dataset JSON files
file(GLOB TEST_DATASETS ${CMAKE_CURRENT_SOURCE_DIR}/data/*.json)

# Custom command: run generator when JSON or script changes
add_custom_command(
    OUTPUT ${GEN_STAMP_FILE}
    COMMAND ${CMAKE_COMMAND} -E echo "Regenerating integration stress-tests from JSON datasets..."
    COMMAND ${Python3_EXECUTABLE} ${GEN_TEST_SCRIPT}
    COMMAND ${CMAKE_COMMAND} -E touch ${GEN_STAMP_FILE}
    DEPENDS ${GEN_TEST_SCRIPT} ${TEST_DATASETS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running gen_tests.py because inputs changed"
)

# Custom target for manual regeneration
add_custom_target(regen-tests
    DEPENDS ${GEN_STAMP_FILE}
)

add_dependencies(runner regen-tests)