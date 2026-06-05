#include "hal_i2c.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>

/* -------------------------------------------------------------------------
 * Clock helpers
 * ------------------------------------------------------------------------- */

static void enable_gpio_clock(uint32_t port)
{
    switch (port) {
        case GPIOA: rcc_periph_clock_enable(RCC_GPIOA); break;
        case GPIOB: rcc_periph_clock_enable(RCC_GPIOB); break;
        case GPIOC: rcc_periph_clock_enable(RCC_GPIOC); break;
        case GPIOD: rcc_periph_clock_enable(RCC_GPIOD); break;
        default: break;
    }
}

static void enable_i2c_clock(uint32_t i2c)
{
    switch (i2c) {
        case I2C1: rcc_periph_clock_enable(RCC_I2C1); break;
        case I2C2: rcc_periph_clock_enable(RCC_I2C2); break;
        case I2C3: rcc_periph_clock_enable(RCC_I2C3); break;
        default: break;
    }
}

/* -------------------------------------------------------------------------
 * hal_i2c_init
 *
 * Configured for 100 kHz (standard mode) with APB1 = 16 MHz (HSI, no PLL):
 *   CCR   = 16 MHz / (2 × 100 kHz) = 80
 *   TRISE = (1000 ns × 16 MHz) + 1  = 17
 * ------------------------------------------------------------------------- */

void hal_i2c_init(HAL_I2C_Handle *h)
{
    enable_gpio_clock(h->gpio_port);
    enable_i2c_clock(h->i2c);

    /* I2C lines must be open-drain; no pull-up here (use external resistors) */
    uint16_t pins = h->gpio_scl | h->gpio_sda;
    gpio_mode_setup(h->gpio_port, GPIO_MODE_AF, GPIO_PUPD_NONE, pins);
    gpio_set_output_options(h->gpio_port, GPIO_OTYPE_OD, GPIO_OSPEED_2MHZ, pins);
    gpio_set_af(h->gpio_port, h->gpio_af, pins);

    i2c_peripheral_disable(h->i2c);
    i2c_set_clock_frequency(h->i2c, 16);
    i2c_set_standard_mode(h->i2c);
    i2c_set_ccr(h->i2c, 80);
    i2c_set_trise(h->i2c, 17);
    i2c_peripheral_enable(h->i2c);
}

/* -------------------------------------------------------------------------
 * hal_i2c_write
 * ------------------------------------------------------------------------- */

bool hal_i2c_write(HAL_I2C_Handle *h, uint8_t addr7, const uint8_t *buf, size_t len)
{
    uint32_t timeout;

    /* Wait until bus is free */
    timeout = 100000;
    while ((I2C_SR2(h->i2c) & I2C_SR2_BUSY) && --timeout);
    if (!timeout) return false;

    /* Generate START */
    i2c_send_start(h->i2c);

    timeout = 100000;
    while (!(I2C_SR1(h->i2c) & I2C_SR1_SB) && --timeout);
    if (!timeout) return false;

    /* Send 7-bit address + WRITE — clears SB */
    i2c_send_7bit_address(h->i2c, addr7, I2C_WRITE);

    /* Wait for ADDR; bail on NACK */
    timeout = 100000;
    while (--timeout) {
        uint32_t sr1 = I2C_SR1(h->i2c);
        if (sr1 & I2C_SR1_ADDR) break;
        if (sr1 & I2C_SR1_AF) {
            i2c_send_stop(h->i2c);
            return false;
        }
    }
    if (!timeout) { i2c_send_stop(h->i2c); return false; }

    /* Reading SR2 clears the ADDR flag and starts the data phase */
    (void)I2C_SR2(h->i2c);

    for (size_t i = 0; i < len; i++) {
        timeout = 100000;
        while (!(I2C_SR1(h->i2c) & I2C_SR1_TxE) && --timeout);
        if (!timeout) { i2c_send_stop(h->i2c); return false; }
        i2c_send_data(h->i2c, buf[i]);
    }

    /* Wait for shift register to finish before issuing STOP */
    timeout = 100000;
    while (!(I2C_SR1(h->i2c) & I2C_SR1_BTF) && --timeout);

    i2c_send_stop(h->i2c);
    return true;
}
