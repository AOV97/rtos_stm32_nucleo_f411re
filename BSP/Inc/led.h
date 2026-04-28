#ifndef LED_H_
#define LED_H_

#include <stdint.h>

/* Nucleo-64 (STM32F411RE): one user LED on PA5 */
#define LED_GREEN 5

void led_init_all(void);
void led_on(uint8_t led_no);
void led_off(uint8_t led_no);

#endif /* LED_H_ */
