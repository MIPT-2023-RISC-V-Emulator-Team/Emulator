#!/bin/bash

args=("$@")

for src_file in "${args[@]}"
do
    src_file_no_ext=${src_file%.c}
    riscv64-unknown-elf-gcc -S  ${src_file_no_ext}.c -o ${src_file_no_ext}.s
    riscv64-unknown-elf-as      ${src_file_no_ext}.s -o ${src_file_no_ext}.o
    riscv64-unknown-elf-ld      ${src_file_no_ext}.o -o ${src_file_no_ext}.exe --entry=main

    rm ${src_file_no_ext}.s
    rm ${src_file_no_ext}.o
done