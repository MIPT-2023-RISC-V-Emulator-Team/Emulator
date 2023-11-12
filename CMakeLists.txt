cmake_minimum_required(VERSION 3.10)

project(RISC-V VERSION 1.0 DESCRIPTION "RISC-V Emulator")

set(CMAKE_CXX_STANDARD 17)
set(COMPILE_OPT -std=c++17 -O3)
set(LINK_OPT -lelf)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

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
file(GLOB_RECURSE EMULATOR_FILES *cpp *.h)
filter_items(EMULATOR_FILES "CMakeFiles/*")

find_program(CLANG_FORMAT "clang-format" REQUIRED)

add_custom_target(format
    COMMAND clang-format
    -i
    -style=file
    ${EMULATOR_FILES}
)

set(ISA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/isa/risc-v.yaml)
set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

add_custom_command(
  OUTPUT ${GEN_DIR}/Decoder.cpp ${GEN_DIR}/Fields.h ${GEN_DIR}/Inctructions.h ${GEN_DIR}/InstructionTypes.h
  COMMAND ruby ${SRC_DIR}/decoder.rb --isa=${ISA_FILE} --gen=${GEN_DIR}
  DEPENDS ${ISA_FILE} ${SRC_DIR}/decoder.rb
  VERBATIM
)

add_custom_command(
  OUTPUT ${GEN_DIR}/Dispatcher.cpp
  COMMAND ruby ${SRC_DIR}/dispatcher.rb --isa=${ISA_FILE} --gen=${GEN_DIR}
  DEPENDS ${ISA_FILE} ${SRC_DIR}/dispatcher.rb
  VERBATIM
)

add_executable(risc-v
    ${SRC_DIR}/risc-v.cpp
    ${SRC_DIR}/Hart.cpp
    ${SRC_DIR}/Memory.cpp
    ${SRC_DIR}/Debug.cpp
    ${SRC_DIR}/ElfLoader.cpp
    ${SRC_DIR}/MMU.cpp
    ${GEN_DIR}/Decoder.cpp
    ${GEN_DIR}/Dispatcher.cpp
)

target_include_directories(risc-v
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC ${INCLUDE_DIR}
)

target_link_libraries(risc-v PUBLIC ${LINK_OPT})
add_compile_options(risc-v ${COMPILE_OPT})
