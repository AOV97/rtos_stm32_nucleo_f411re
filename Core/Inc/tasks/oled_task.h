#ifndef OLED_TASK_H_
#define OLED_TASK_H_

/**
 * @brief FreeRTOS task that drives the SSD1306 OLED display over I2C.
 *
 * Calls bsp_oled_i2c_init() to set up I2C1, initializes the SSD1306 driver,
 * then loops every 500 ms: clears the framebuffer, draws static labels and a
 * live tick counter, and calls ssd1306_flush() to push the frame to the display.
 * All I2C and display state is owned internally — no shared handles needed.
 *
 * @param pvParameters  Unused. Pass NULL when creating with xTaskCreate.
 */
void oled_task(void *pvParameters);

#endif /* OLED_TASK_H_ */
