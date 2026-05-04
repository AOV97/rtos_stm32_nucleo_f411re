#ifndef PLATFORM_CONFIG_H_
#define PLATFORM_CONFIG_H_

#include <libopencm3/stm32/gpio.h>

/* Onboard user LED — LD2 on PA5 */
#define LED_PORT    GPIOA
#define LED_PIN     GPIO5

#endif /* PLATFORM_CONFIG_H_ */
