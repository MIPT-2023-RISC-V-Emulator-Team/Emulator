cmake_minimum_required(VERSION 3.10)

set(ASMJIT_EMBED TRUE)

add_definitions(-DASMJIT_STATIC)

# TODO(panferovi): find universal way to integrate with third-party
set(ASMJIT_DIR ${SRC_DIR}/asmjit)
include("${ASMJIT_DIR}/CMakeLists.txt")

set(COMPILER_SRC
    Compiler.cpp
    CompilerWorker.cpp
)

add_library(compiler
    ${COMPILER_SRC}
    ${ASMJIT_SRC}
)

target_include_directories(compiler
    PUBLIC ${SRC_DIR}
    PUBLIC ${BIN_DIR}
)

target_link_libraries(compiler PRIVATE ${ASMJIT_DEPS})
