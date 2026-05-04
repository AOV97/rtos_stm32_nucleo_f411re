#include "FreeRTOS.h"
#include "task.h"
#include "cbuf.h"

void producer_task(void *params)
{
    CircBuf_t *cbuf = (CircBuf_t *)params;
    uint8_t cmd = 1;
    while (1)
    {
        cbuf_write(cbuf, cmd);
        cmd ^= 1u;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
