# Integration test executable
add_executable(runner runner.cpp)
target_link_libraries(runner PRIVATE minisql)

# Dataset generator script + stamp
set(GEN_DATA_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/gen_data.py)
set(GEN_DATA_STAMP ${CMAKE_CURRENT_BINARY_DIR}/gen_data.stamp)

# Schema for generated datasets
set(DATASETS_SCHEMA ${CMAKE_CURRENT_SOURCE_DIR}/data/schema.json)

# Run datasets generator when script or schema changes
add_custom_command(
    OUTPUT ${GEN_DATA_STAMP}
    COMMAND ${CMAKE_COMMAND} -E echo "Regenerating datasets for integration stress-tests..."
    COMMAND ${Python3_EXECUTABLE} ${GEN_DATA_SCRIPT}
    COMMAND ${CMAKE_COMMAND} -E touch ${GEN_DATA_STAMP}
    DEPENDS ${GEN_DATA_SCRIPT} ${DATASETS_SCHEMA}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running gen_data.py to produce datasets"
)

# Custom target for manual data regeneration
add_custom_target(regen-data DEPENDS ${GEN_DATA_STAMP})

# SQL/OUT generator script + stamp
set(GEN_TEST_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/gen_tests.py)
set(GEN_TEST_STAMP ${CMAKE_CURRENT_BINARY_DIR}/gen_tests.stamp)

# Run tests generator when datasets or script changes
add_custom_command(
    OUTPUT ${GEN_TEST_STAMP}
    COMMAND ${CMAKE_COMMAND} -E echo "Regenerating integration stress-tests from datasets..."
    COMMAND ${Python3_EXECUTABLE} ${GEN_TEST_SCRIPT}
    COMMAND ${CMAKE_COMMAND} -E touch ${GEN_TEST_STAMP}
    DEPENDS ${GEN_TEST_SCRIPT} ${GEN_DATA_STAMP}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running gen_tests.py to produce stress-tests"
)

# Custom target for manual tests regeneration
add_custom_target(regen-tests DEPENDS ${GEN_TEST_STAMP})

# Custom target for manual total regeneration
add_custom_target(regen-all DEPENDS gen-data regen-tests)

add_dependencies(runner regen-tests)