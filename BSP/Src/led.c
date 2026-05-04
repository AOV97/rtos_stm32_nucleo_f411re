#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "led.h"
#include "platform_config.h"

void led_init_all(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(LED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_PIN);
    gpio_set_output_options(LED_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, LED_PIN);
    gpio_clear(LED_PORT, LED_PIN);
}

void led_on(uint8_t led_no)
{
    (void)led_no;
    gpio_set(LED_PORT, LED_PIN);
}

void led_off(uint8_t led_no)
{
    (void)led_no;
    gpio_clear(LED_PORT, LED_PIN);
}
