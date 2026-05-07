cmake_minimum_required(VERSION 3.20)

add_library(hal STATIC
    Src/hal_spi.c
)

target_include_directories(hal PUBLIC Inc)

target_compile_options(hal PRIVATE ${CPU_FLAGS} -std=gnu11 -Wall -O0 -g)

target_link_libraries(hal PUBLIC opencm3 freertos_kernel bsp)
