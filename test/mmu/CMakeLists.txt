set(TEST_EXEC MMUTests)

set(TEST_SOURCES
    ${SRC_DIR}/simulator/memory/Memory.cpp
    ${SRC_DIR}/simulator/memory/MMU.cpp
    ${TEST_EXEC}.cpp
)


add_executable(${TEST_EXEC} ${TEST_SOURCES})
target_link_libraries(${TEST_EXEC} GTest::gtest_main)

target_include_directories(${TEST_EXEC}
    PUBLIC ${SRC_DIR}
    PUBLIC ${BIN_DIR}
)

add_custom_target(Run_MMU_Tests
    DEPENDS ${TEST_EXEC}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${TEST_EXEC}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running MMU tests"
    VERBATIM
)