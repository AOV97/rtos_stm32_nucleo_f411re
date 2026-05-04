#include "FreeRTOS.h"
#include "task.h"
#include "cbuf.h"
#include "led.h"

void consumer_task(void *params)
{
    CircBuf_t *cbuf = (CircBuf_t *)params;
    while (1)
    {
        uint8_t cmd = cbuf_read(cbuf);
        if (cmd)
            led_on(LED_GREEN);
        else
            led_off(LED_GREEN);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
