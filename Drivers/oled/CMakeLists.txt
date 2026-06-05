cmake_minimum_required(VERSION 3.20)

add_library(oled STATIC
    Src/ssd1306.c
)

target_include_directories(oled PUBLIC Inc)

target_compile_options(oled PRIVATE ${CPU_FLAGS} -std=gnu11 -Wall -O0 -g)

target_link_libraries(oled PUBLIC hal)
