/**
 ******************************************************************************
 * @file    main.c
 * @brief   FreeRTOS LED blink demo for STM32F411RE (Nucleo-64)
 *
 * Four tasks blink the on-board LEDs at different rates using vTaskDelay,
 * which is identical in behaviour to the hand-rolled scheduler that was
 * here before — but now FreeRTOS manages context switching, stacks, and
 * timing for you.
 *
 * LED pin mapping (GPIO D):
 *   GREEN  -> PD12  task1  1 Hz  (1000 ms on/off)
 *   ORANGE -> PD13  task2  1 Hz  (1000 ms on/off)
 *   BLUE   -> PD15  task3  4 Hz  ( 250 ms on/off)
 *   RED    -> PD14  task4  8 Hz  ( 125 ms on/off)
 ******************************************************************************
 */

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "led.h"

/* Task function prototypes */
static void task1_handler(void *params);
static void task2_handler(void *params);
static void task3_handler(void *params);
static void task4_handler(void *params);

/* -------------------------------------------------------------------------
 * Fault handlers
 * FreeRTOS port overrides SVC_Handler, PendSV_Handler and SysTick_Handler
 * via the macros in FreeRTOSConfig.h.  The handlers below are for the
 * remaining faults; they remain as infinite loops so the debugger stops here.
 * -------------------------------------------------------------------------*/
void HardFault_Handler(void)  { while(1); }
void MemManage_Handler(void)  { while(1); }
void BusFault_Handler(void)   { while(1); }
void UsageFault_Handler(void) { while(1); }
void NMI_Handler(void)        { while(1); }

/* Enable Memory Management, Bus Fault, and Usage Fault exceptions so they
 * are reported individually instead of escalating to HardFault. */
static void enable_processor_faults(void)
{
    uint32_t *pSHCSR = (uint32_t*)0xE000ED24;
    *pSHCSR |= (1u << 16); /* MemManage  */
    *pSHCSR |= (1u << 17); /* BusFault   */
    *pSHCSR |= (1u << 18); /* UsageFault */
}

/* -------------------------------------------------------------------------
 * main
 * -------------------------------------------------------------------------*/
int main(void)
{
    enable_processor_faults();

    led_init_all();

    /* Create the four LED tasks.  All run at the same priority (2).
     * configMINIMAL_STACK_SIZE (128 words = 512 bytes) is enough for these
     * simple tasks — raise it if you add printf / heavy locals. */
    xTaskCreate(task1_handler, "Task1", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(task2_handler, "Task2", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(task3_handler, "Task3", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(task4_handler, "Task4", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

    /* Hand control to the FreeRTOS scheduler.  Does not return. */
    vTaskStartScheduler();

    /* Should never reach here.  If it does, there was not enough heap to
     * create the idle task — increase configTOTAL_HEAP_SIZE. */
    for(;;);
}

/* -------------------------------------------------------------------------
 * Task implementations
 * -------------------------------------------------------------------------*/
static void task1_handler(void *params)
{
    (void)params;
    while(1)
    {
        led_on(LED_GREEN);
        vTaskDelay(pdMS_TO_TICKS(1000));
        led_off(LED_GREEN);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void task2_handler(void *params)
{
    (void)params;
    while(1)
    {
        led_on(LED_ORANGE);
        vTaskDelay(pdMS_TO_TICKS(1000));
        led_off(LED_ORANGE);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void task3_handler(void *params)
{
    (void)params;
    while(1)
    {
        led_on(LED_BLUE);
        vTaskDelay(pdMS_TO_TICKS(250));
        led_off(LED_BLUE);
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

static void task4_handler(void *params)
{
    (void)params;
    while(1)
    {
        led_on(LED_RED);
        vTaskDelay(pdMS_TO_TICKS(125));
        led_off(LED_RED);
        vTaskDelay(pdMS_TO_TICKS(125));
    }
}
