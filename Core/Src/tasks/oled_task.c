#include "tasks/oled_task.h"
#include "ssd1306.h"
#include "bsp.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

void oled_task(void *pvParameters)
{
    (void)pvParameters;

    HAL_I2C_Handle i2c;
    bsp_oled_i2c_init(&i2c);

    SSD1306_Handle oled;
    ssd1306_init(&oled, &i2c, BSP_OLED_I2C_ADDR);

    char line[22];
    uint32_t tick = 0;

    for (;;) {
        ssd1306_clear(&oled);

        ssd1306_draw_string(&oled, 0, 0, "STM32F411 + RTOS");
        ssd1306_draw_string(&oled, 0, 2, "I2C SSD1306 128x64");

        snprintf(line, sizeof(line), "Tick: %lu", tick++);
        ssd1306_draw_string(&oled, 0, 4, line);

        ssd1306_draw_string(&oled, 0, 6, "Page 6 ready");

        ssd1306_flush(&oled);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
