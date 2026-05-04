#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "led.h"
#include "cbuf.h"
#include "tasks/producer_task.h"
#include "tasks/consumer_task.h"

/* OpenOCD FreeRTOS thread-awareness looks for this symbol — newer FreeRTOS
 * removed it, so we provide it manually. */
const volatile uint32_t uxTopUsedPriority = configMAX_PRIORITIES - 1;

static CircBuf_t g_cbuf;

int main(void)
{
    bsp_init();
    led_init_all();
    cbuf_init(&g_cbuf);

    xTaskCreate(producer_task, "Producer", configMINIMAL_STACK_SIZE, &g_cbuf, 2, NULL);
    xTaskCreate(consumer_task, "Consumer", configMINIMAL_STACK_SIZE, &g_cbuf, 2, NULL);

    vTaskStartScheduler();
    for (;;);
}
