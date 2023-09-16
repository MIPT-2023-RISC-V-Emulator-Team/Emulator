cmake_minimum_required(VERSION 3.10)

project(RISC-V VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(SRC_DIR ./src)
set(INCLUDE_DIR ./include)
set(COMPILE_OPT -std=c++17 -O2)
set(LINK_OPT -lelf)


add_executable(
	risc-v.exe 

	${SRC_DIR}/risc-v.cpp
	${SRC_DIR}/CpuRV.cpp
	${SRC_DIR}/Memory.cpp

	${INCLUDE_DIR}/CpuRV.h
	${INCLUDE_DIR}/Memory.h
)

target_link_libraries(risc-v.exe PUBLIC ${LINK_OPT})
add_compile_options(risc-v.exe ${COMPILE_OPT})
