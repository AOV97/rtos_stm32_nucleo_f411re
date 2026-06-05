cmake_minimum_required(VERSION 3.20)

set(LOG_LEVEL "LOG_LVL_DEBUG" CACHE STRING "Minimum log level to compile in (LOG_LVL_DEBUG/INFO/WARNING/ERROR/NONE)")

add_library(core STATIC
    Src/main.c
    Src/cbuf.c
    Src/tasks/producer_task.c
    Src/tasks/consumer_task.c
    Src/tasks/sd_task.c
    Src/tasks/oled_task.c
    Src/logger.c
)

target_include_directories(core PUBLIC Inc)
target_include_directories(core PRIVATE ${CMAKE_SOURCE_DIR}/BSP/${BOARD})

target_compile_options(core PRIVATE ${CPU_FLAGS} -std=gnu11 -Wall -O0 -g)
target_compile_definitions(core PUBLIC LOG_LEVEL=${LOG_LEVEL})

target_link_libraries(core PUBLIC freertos_kernel bsp sd_card oled)
