#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "bsp.h"
#include "logger.h"
#include "tasks/oled_task.h"

/* OpenOCD FreeRTOS thread-awareness looks for this symbol — newer FreeRTOS
 * removed it, so we provide it manually. */
const volatile uint32_t uxTopUsedPriority = configMAX_PRIORITIES - 1;

int main(void)
{
    bsp_init();
    logger_init();

    xTaskCreate(logger_task, "Logger", configMINIMAL_STACK_SIZE * 4, NULL, 1, NULL);
    xTaskCreate(oled_task,   "OLED",   configMINIMAL_STACK_SIZE * 4, NULL, 1, NULL);

    LOG_DEBUG("Main", "System initialized, starting scheduler");
    LOG_INFO("Main", "FreeRTOS version: %s", tskKERNEL_VERSION_NUMBER);
    LOG_WARNING("Main", "This is a warning message");
    LOG_ERROR("Main", "This is an error message");

    vTaskStartScheduler();
    for (;;);
}
