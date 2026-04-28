#ifndef STM32F4XX_HAL_CONF_H
#define STM32F4XX_HAL_CONF_H

/* ---- Enable only what we use ------------------------------------------- */
#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED  /* required by HAL core */
#define HAL_GPIO_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED   /* header only — macros used inside RCC driver */

/* ---- Oscillator values (chip defaults, no external crystal on Nucleo) ---- */
#define HSE_VALUE    8000000U   /* 8 MHz from ST-LINK MCO on Nucleo */
#define HSE_STARTUP_TIMEOUT 100U
#define HSI_VALUE   16000000U   /* 16 MHz internal RC — our actual clock */
#define LSI_VALUE      32000U
#define LSE_VALUE      32768U
#define LSE_STARTUP_TIMEOUT 5000U
#define EXTERNAL_CLOCK_VALUE 12288000U

/* ---- Tick interrupt priority (unused — FreeRTOS owns SysTick) ----------- */
#define TICK_INT_PRIORITY 0x0FU

/* ---- Pull in the HAL include chain -------------------------------------- */
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_rcc_ex.h"

/* ---- assert_param: disabled in release builds --------------------------- */
#define assert_param(expr) ((void)0U)

#endif /* STM32F4XX_HAL_CONF_H */
