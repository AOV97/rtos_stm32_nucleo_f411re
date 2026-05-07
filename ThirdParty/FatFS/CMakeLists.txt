cmake_minimum_required(VERSION 3.20)

add_library(fatfs STATIC ff.c)

target_include_directories(fatfs PUBLIC .)

target_compile_options(fatfs PRIVATE ${CPU_FLAGS} -std=gnu11 -O2)
