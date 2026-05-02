#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "led.h"

void led_init_all(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO5);
    gpio_clear(GPIOA, GPIO5);
}

void led_on(uint8_t led_no)
{
    (void)led_no;
    gpio_set(GPIOA, GPIO5);
}

void led_off(uint8_t led_no)
{
    (void)led_no;
    gpio_clear(GPIOA, GPIO5);
}
