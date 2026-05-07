cmake_minimum_required(VERSION 3.20)

add_library(sd_card STATIC
    Src/sd_card.c
    Src/diskio.c
    ${CMAKE_SOURCE_DIR}/ThirdParty/FatFS/ff.c
)

target_include_directories(sd_card PUBLIC
    Inc
    ${CMAKE_SOURCE_DIR}/ThirdParty/FatFS
)

target_compile_options(sd_card PRIVATE ${CPU_FLAGS} -std=gnu11 -Wall -O0 -g)

target_link_libraries(sd_card PUBLIC hal bsp)
