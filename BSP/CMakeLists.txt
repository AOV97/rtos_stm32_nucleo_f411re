cmake_minimum_required(VERSION 3.20)

add_library(bsp STATIC Src/led.c)

target_include_directories(bsp PUBLIC Inc)

target_compile_options(bsp PRIVATE ${CPU_FLAGS} -std=gnu11 -Wall -O0 -g)