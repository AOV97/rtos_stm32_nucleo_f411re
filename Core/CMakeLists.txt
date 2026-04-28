cmake_minimum_required(VERSION 3.20)

add_library(core STATIC Src/main.c Src/cbuf.c)

target_include_directories(core PUBLIC Inc)

target_compile_options(core PRIVATE ${CPU_FLAGS} -std=gnu11 -Wall -O0 -g)

target_link_libraries(core PUBLIC freertos_kernel bsp)
