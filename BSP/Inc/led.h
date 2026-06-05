#ifndef LED_H_
#define LED_H_

#include <stdint.h>

/* Nucleo-64 (STM32F411RE): one user LED on PA5 */
#define LED_GREEN 5

/**
 * @brief Configures all user LED GPIO pins as push-pull outputs.
 *
 * Must be called once during board initialization before any led_on / led_off
 * calls. Sets each LED pin to output mode with no pull-up/pull-down and
 * drives them low (LEDs off) as the initial state.
 */
void led_init_all(void);

/**
 * @brief Turns on the specified user LED.
 *
 * @param led_no  LED identifier. Use the LED_GREEN (5) define for the Nucleo
 *                onboard green LED on PA5. Passing an unrecognised value is a
 *                no-op.
 */
void led_on(uint8_t led_no);

/**
 * @brief Turns off the specified user LED.
 *
 * @param led_no  LED identifier. Use the LED_GREEN (5) define for the Nucleo
 *                onboard green LED on PA5. Passing an unrecognised value is a
 *                no-op.
 */
void led_off(uint8_t led_no);

#endif /* LED_H_ */
