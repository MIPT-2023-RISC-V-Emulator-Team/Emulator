cmake_minimum_required(VERSION 3.10)

project(RISC-V VERSION 1.0 DESCRIPTION "RISC-V Emulator")

set(LINK_OPT -lelf)

set(ISA_FILE ${SRC_DIR}/isa/risc-v.yaml)
set(GEN_DIR ${BIN_DIR}/generated)

set(DECODER_OUTPUT
    ${GEN_DIR}/Fields.h
    ${GEN_DIR}/Inctructions.h
    ${GEN_DIR}/InstructionTypes.h
    ${GEN_DIR}/Decoder.cpp
)

execute_process(
    COMMAND ruby ${CMAKE_CURRENT_SOURCE_DIR}/decoder.rb --isa=${ISA_FILE} --gen=${GEN_DIR}
    DEPENDS ${ISA_FILE} decoder.rb
    VERBATIM
)

execute_process(
    COMMAND ruby ${CMAKE_CURRENT_SOURCE_DIR}/dispatcher.rb --isa=${ISA_FILE} --gen=${GEN_DIR}
    DEPENDS ${ISA_FILE} dispatcher.rb
    VERBATIM
)

set(SIMULATOR_SRC
    Hart.cpp
    OSHelper.cpp
    memory/Memory.cpp
    memory/MMU.cpp
    ${GEN_DIR}/Decoder.cpp
    ${GEN_DIR}/Dispatcher.cpp
)

add_library(simulator ${SIMULATOR_SRC})

target_include_directories(simulator
    PUBLIC ${SRC_DIR}
    PUBLIC ${BIN_DIR}
)

target_link_libraries(simulator PRIVATE ${LINK_OPT})

target_compile_options(simulator PUBLIC -Wno-invalid-offsetof)
