#!/bin/bash

script_dir=$(dirname "$(realpath $0)")

riscv64-unknown-elf-gcc -S  ${script_dir}/test/test.c -o ${script_dir}/test/test.s
riscv64-unknown-elf-as      ${script_dir}/test/test.s -o ${script_dir}/test/test.o
riscv64-unknown-elf-ld      ${script_dir}/test/test.o -o ${script_dir}/test/test.exe --entry=main

rm ${script_dir}/test/test.s
rm ${script_dir}/test/test.o
