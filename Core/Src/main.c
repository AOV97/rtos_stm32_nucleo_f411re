/**
 ******************************************************************************
 * @file    main.c
 * @brief   Circular-buffer producer/consumer demo for STM32F411RE (Nucleo-64)
 *
 * LD2 (PA5, green) is driven by a producer/consumer pair sharing a CircBuf_t:
 *   Producer  writes ON/OFF commands every 100 ms  (10 items/sec)
 *   Consumer  reads  one command   every 500 ms  ( 2 items/sec)
 *
 * The 8-slot buffer fills after ~800 ms.  Once full, the producer blocks on
 * sem_free until the consumer frees a slot.  LD2 then blinks at the consumer
 * rate (500 ms/toggle) instead of the faster producer rate.
 ******************************************************************************
 */

#include <stdint.h>
#include <libopencm3/cm3/scb.h>
#include "FreeRTOS.h"
#include "task.h"
#include "led.h"
#include "cbuf.h"

/* OpenOCD FreeRTOS thread-awareness looks for this symbol — newer FreeRTOS
 * removed it, so we provide it manually. */
const volatile uint32_t uxTopUsedPriority = configMAX_PRIORITIES - 1;

/* Shared circular buffer — initialised in main before tasks are created */
static CircBuf_t g_cbuf;

static void producer_task(void *params);
static void consumer_task(void *params);

/* -------------------------------------------------------------------------
 * Fault handlers
 * -------------------------------------------------------------------------*/
void HardFault_Handler(void)  { while(1); }
void MemManage_Handler(void)  { while(1); }
void BusFault_Handler(void)   { while(1); }
void UsageFault_Handler(void) { while(1); }
void NMI_Handler(void)        { while(1); }

static void enable_processor_faults(void)
{
    uint32_t *pSHCSR = (uint32_t*)0xE000ED24;
    *pSHCSR |= (1u << 16); /* MemManage  */
    *pSHCSR |= (1u << 17); /* BusFault   */
    *pSHCSR |= (1u << 18); /* UsageFault */
}

/* Enable FPU coprocessors CP10 and CP11.
 * Must run before vTaskStartScheduler() — the CM4F port saves/restores
 * FPU registers on every context switch and will hard-fault if CP10/11 are off. */
static void fpu_enable(void)
{
    SCB_CPACR |= (3u << 20) | (3u << 22);
    __asm volatile("dsb" ::: "memory");
    __asm volatile("isb" ::: "memory");
}

/* -------------------------------------------------------------------------
 * main
 * -------------------------------------------------------------------------*/
int main(void)
{
    fpu_enable();
    enable_processor_faults();
    led_init_all();

    cbuf_init(&g_cbuf);

    xTaskCreate(producer_task, "Producer", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(consumer_task, "Consumer", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

    vTaskStartScheduler();
    for(;;);
}

/* -------------------------------------------------------------------------
 * Producer: writes ON/OFF commands at 100 ms intervals.
 * Blocks in cbuf_write() once the 8-slot buffer is full.
 * -------------------------------------------------------------------------*/
static void producer_task(void *params)
{
    (void)params;
    uint8_t cmd = 1;
    while (1)
    {
        cbuf_write(&g_cbuf, cmd);
        cmd ^= 1u;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/* -------------------------------------------------------------------------
 * Consumer: reads one command every 500 ms (deliberately slow).
 * Blocks in cbuf_read() when the buffer is empty.
 * -------------------------------------------------------------------------*/
static void consumer_task(void *params)
{
    (void)params;
    while (1)
    {
        uint8_t cmd = cbuf_read(&g_cbuf);
        if (cmd)
            led_on(LED_GREEN);
        else
            led_off(LED_GREEN);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
