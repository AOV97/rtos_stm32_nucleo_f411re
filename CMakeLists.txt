cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/arm-none-eabi-gcc.cmake)
project(stm32RTOS C ASM)

set(FREERTOS_DIR ThirdParty/FreeRTOS)

# Processor/ABI flags - same as you CFLAGS, but as a list
set(CPU_FLAGS
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
)

# FreeRTOS configuration — must be set before add_subdirectory
set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "FreeRTOS port")
set(FREERTOS_HEAP 4            CACHE STRING "FreeRTOS heap implementation (1-5)")

# INTERFACE library that tells FreeRTOS where to find FreeRTOSConfig.h
# CPU_FLAGS must be passed here so the CM4F port sees -mfpu / -mfloat-abi=hard
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE Core/Inc)
target_compile_options(freertos_config INTERFACE ${CPU_FLAGS})

add_subdirectory(ThirdParty/FreeRTOS)
add_subdirectory(BSP)
add_subdirectory(Core)

add_executable(firmware.elf Startup/stm32_startup.c)
target_compile_options(firmware.elf PRIVATE ${CPU_FLAGS} -std=gnu11 -O0 -g)
target_link_libraries(firmware.elf PRIVATE core bsp freertos_kernel)
target_link_options(firmware.elf PRIVATE
    ${CPU_FLAGS}
    -nostdlib                                                                                         
    -T ${CMAKE_SOURCE_DIR}/Linker/stm32_ls.ld
    -Wl,-Map=${CMAKE_BINARY_DIR}/firmware.map                                                         
    -L /usr/lib/arm-none-eabi/newlib/thumb/v7e-m+fp/hard
)                                                                                                     
target_link_libraries(firmware.elf PRIVATE c gcc)
                                                                                                    
add_custom_command(TARGET firmware.elf POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:firmware.elf>
)     

add_custom_target(load
      COMMAND openocd
          -f /usr/share/openocd/scripts/interface/stlink-v2-1.cfg
          -f /usr/share/openocd/scripts/target/stm32f4x.cfg
      USES_TERMINAL
  )