cmake_minimum_required(VERSION 3.20)

set(HAL_DRIVER_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/../stm32f4xx_hal_driver)
set(CMSIS_DEVICE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cmsis_device_f4)
set(CMSIS_CORE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/../CMSIS/CMSIS/Core/Include)

# Only pull in the modules we actually use.
# Adding a new peripheral later = add one .c here.
set(HAL_SOURCES
    ${HAL_DRIVER_DIR}/Src/stm32f4xx_hal.c
    ${HAL_DRIVER_DIR}/Src/stm32f4xx_hal_cortex.c
    ${HAL_DRIVER_DIR}/Src/stm32f4xx_hal_gpio.c
    ${HAL_DRIVER_DIR}/Src/stm32f4xx_hal_rcc.c
    ${HAL_DRIVER_DIR}/Src/stm32f4xx_hal_rcc_ex.c
    ${CMSIS_DEVICE_DIR}/Source/Templates/system_stm32f4xx.c
)

add_library(stm32f4xx_hal STATIC ${HAL_SOURCES})

target_include_directories(stm32f4xx_hal PUBLIC
    ${HAL_DRIVER_DIR}/Inc
    ${CMSIS_DEVICE_DIR}/Include
    ${CMSIS_CORE_DIR}
    ${CMAKE_SOURCE_DIR}/Core/Inc   # stm32f4xx_hal_conf.h lives here
)

target_compile_definitions(stm32f4xx_hal PUBLIC
    USE_HAL_DRIVER
    STM32F411xE
)

target_compile_options(stm32f4xx_hal PRIVATE ${CPU_FLAGS} -std=gnu11 -Wall -O0 -g)
