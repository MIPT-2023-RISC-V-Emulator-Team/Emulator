cmake_minimum_required(VERSION 3.10)

project(RISC-V VERSION 1.0 DESCRIPTION "RISC-V Emulator")

set(CMAKE_CXX_STANDARD 17)
set(COMPILE_OPT -std=c++17 -O3)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

# Remove strings matching given regular expression from a list.
# @param(in,out) aItems Reference of a list variable to filter.
# @param aRegEx Value of regular expression to match.
function (filter_items aItems aRegEx)
    # For each item in our list
    foreach (item ${${aItems}})
        # Check if our items matches our regular expression
        if ("${item}" MATCHES ${aRegEx})
            # Remove current item from our list
            list (REMOVE_ITEM ${aItems} ${item})
        endif ("${item}" MATCHES ${aRegEx})
    endforeach(item)
    # Provide output parameter
    set(${aItems} ${${aItems}} PARENT_SCOPE)
endfunction (filter_items)

file(GLOB_RECURSE EMULATOR_FILES 
    simulator/*.cpp
    simulator/*.h
    compiler/*.cpp
    compiler/*.h
    utils/*.cpp
    utils/*.h
)


filter_items(EMULATOR_FILES "CMakeFiles/*")

find_program(CLANG_FORMAT "clang-format" REQUIRED)

add_custom_target(format
    COMMAND clang-format
    -i
    -style=file
    ${EMULATOR_FILES}
)

add_subdirectory(googletest)
add_subdirectory(simulator)
add_subdirectory(compiler)
add_subdirectory(test/mmu)

add_library(utils utils/Debug.cpp)

add_executable(risc-v riscv/risc-v.cpp)

target_include_directories(risc-v
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(risc-v
    PUBLIC simulator
    PUBLIC compiler
    PUBLIC utils
)

add_compile_options(risc-v ${COMPILE_OPT})
