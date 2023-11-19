cmake_minimum_required(VERSION 3.10)

project(RISC-V VERSION 1.0 DESCRIPTION "RISC-V Emulator")

set(COMPILER_SRC
    Compiler.cpp
    CompilerWorker.cpp
)

add_library(compiler ${COMPILER_SRC})

target_include_directories(compiler
    PUBLIC ${SRC_DIR}
    PUBLIC ${BIN_DIR}
)
