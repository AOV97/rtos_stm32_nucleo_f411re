cmake_minimum_required(VERSION 3.20)

add_library(startup STATIC stm32_startup.c)

target_compile_options(startup PRIVATE ${CPU_FLAGS} -std=gnu11 -O0 -g) 