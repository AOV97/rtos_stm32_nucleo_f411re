#ifndef HAL_I2C_H_
#define HAL_I2C_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Persistent configuration for one I2C peripheral.
 * Fill all fields, then call hal_i2c_init().
 */
typedef struct {
    uint32_t i2c;        /* I2C1, I2C2, or I2C3 */
    uint32_t gpio_port;  /* GPIO port shared by SCL and SDA */
    uint16_t gpio_scl;
    uint16_t gpio_sda;
    uint8_t  gpio_af;    /* Alternate function number (GPIO_AF4 for I2C1/2/3) */
} HAL_I2C_Handle;

/*
 * Enable clocks, configure GPIO open-drain AF, and set up the I2C peripheral
 * for 100 kHz standard mode assuming a 16 MHz APB1 clock (HSI, no PLL).
 */
void hal_i2c_init(HAL_I2C_Handle *h);

/*
 * Blocking master write: sends `len` bytes from `buf` to 7-bit address `addr7`.
 * Returns true on success, false on NACK or timeout.
 * Safe to call from a FreeRTOS task (uses polling, does not block the scheduler).
 */
bool hal_i2c_write(HAL_I2C_Handle *h, uint8_t addr7, const uint8_t *buf, size_t len);

#endif /* HAL_I2C_H_ */
