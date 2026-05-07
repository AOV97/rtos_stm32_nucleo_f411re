#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "tasks/sd_task.h"

/* OpenOCD FreeRTOS thread-awareness looks for this symbol — newer FreeRTOS
 * removed it, so we provide it manually. */
const volatile uint32_t uxTopUsedPriority = configMAX_PRIORITIES - 1;

int main(void)
{
    bsp_init();

    xTaskCreate(sd_task, "SD", configMINIMAL_STACK_SIZE * 4, NULL, 2, NULL);

    vTaskStartScheduler();
    for (;;);
}
