#include <libopencm3/cm3/scb.h>
#include "bsp.h"

static void fpu_enable(void)
{
    SCB_CPACR |= (3u << 20) | (3u << 22);
    __asm volatile("dsb" ::: "memory");
    __asm volatile("isb" ::: "memory");
}

static void enable_processor_faults(void)
{
    uint32_t *pSHCSR = (uint32_t *)0xE000ED24;
    *pSHCSR |= (1u << 16); /* MemManage  */
    *pSHCSR |= (1u << 17); /* BusFault   */
    *pSHCSR |= (1u << 18); /* UsageFault */
}

void bsp_init(void)
{
    fpu_enable();
    enable_processor_faults();
}
