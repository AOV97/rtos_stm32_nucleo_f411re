#ifndef BSP_H_
#define BSP_H_

#include "hal_i2c.h"

/**
 * @brief Initializes the board support package.
 *
 * Enables the Cortex-M4 FPU (required before vTaskStartScheduler because the
 * ARM_CM4F FreeRTOS port saves/restores FPU registers on every context switch)
 * and activates the MemManage, BusFault, and UsageFault handlers so hard faults
 * surface as distinct fault types rather than escalating to HardFault.
 * Must be the first call in main().
 */
void bsp_init(void);

/**
 * @brief Fills a HAL_I2C_Handle with the board's OLED I2C pin constants and
 *        initializes the I2C1 peripheral.
 *
 * Owns all platform-specific pin/peripheral knowledge for the OLED bus
 * (I2C1, PB8 SCL, PB9 SDA, AF4). After this call the handle is ready for use
 * by the SSD1306 driver. This is the only function in Core/ that ever needs to
 * change when the design is ported to a different board.
 *
 * @param out  Pointer to an uninitialized HAL_I2C_Handle. The function writes
 *             all fields and calls hal_i2c_init() before returning.
 */
void bsp_oled_i2c_init(HAL_I2C_Handle *out);

/* 7-bit I2C address of the SSD1306 OLED module (most common value). */
#define BSP_OLED_I2C_ADDR  0x3C

#endif /* BSP_H_ */
