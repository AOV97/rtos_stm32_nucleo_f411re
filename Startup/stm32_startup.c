#include <stdint.h>

#define SRAM_START 0x20000000u
#define SRAM_SIZE (128u * 1024u) // 128 KB
#define SRAM_END (SRAM_START) + (SRAM_SIZE)

#define STACK_START SRAM_END

extern uint32_t _la_data;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

//prototype of main 
int main(void);

//Function prototype for each IRQ handler

/**
 * @brief System Exception Handlers1
 * These handlers are defined as weak symbols, allowing the user to override them
 * with their own implementations in the application code. If the user does not
 * provide an implementation for a specific handler, the Default_Handler will be used,
 * which typically contains an infinite loop to indicate that an unhandled interrupt has occurred.
 */
void Reset_Handler(void);
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

/**
 * @brief 52 Maskable Interrupt Channels
 * These handlers are also defined as weak symbols, allowing the user to override them
 * with their own implementations in the application code. If the user does not
 * provide an implementation for a specific handler, the Default_Handler will be used,
 * which typically contains an infinite loop to indicate that an unhandled interrupt has occurred.
 */
void WWDG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI16_PVD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI21_Tamp_Stamp_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI22_RTC_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void RCC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ADC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI9_5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_BRK_TIM9_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_UP_TIM10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_TRG_COM_TIM11_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_CC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_EV_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_ER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI15_10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI17_RTC_Alarm_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void EXTI18_OTG_FS_WKUP_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMA1_Stream7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SDIO_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SPI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

/* DMA2 streams — IRQ 54-62 on STM32F411 */
void dma2_stream0_isr(void) __attribute__((weak, alias("Default_Handler")));
void dma2_stream1_isr(void) __attribute__((weak, alias("Default_Handler")));
void dma2_stream2_isr(void) __attribute__((weak, alias("Default_Handler")));
void dma2_stream3_isr(void) __attribute__((weak, alias("Default_Handler")));
void dma2_stream4_isr(void) __attribute__((weak, alias("Default_Handler")));
void dma2_stream5_isr(void) __attribute__((weak, alias("Default_Handler")));
void dma2_stream6_isr(void) __attribute__((weak, alias("Default_Handler")));
void dma2_stream7_isr(void) __attribute__((weak, alias("Default_Handler")));

void otg_fs_isr(void) __attribute__((weak, alias("Default_Handler")));

uint32_t vector[]  __attribute__((section(".isr_vector"))) = {
    /* 16 Core Exceptions */
    STACK_START, // Initial Stack Pointer
    (uint32_t)&Reset_Handler, // Reset Handler
    (uint32_t)&NMI_Handler, // NMI Handler
    (uint32_t)&HardFault_Handler, // Hard Fault Handler
    (uint32_t)&MemManage_Handler, // Memory Management Fault Handler
    (uint32_t)&BusFault_Handler, // Bus Fault Handler
    (uint32_t)&UsageFault_Handler, // Usage Fault Handler
    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // Reserved
    (uint32_t)&SVC_Handler, // SVCall Handler
    (uint32_t)&DebugMon_Handler, // Debug Monitor Handler
    0, // Reserved
    (uint32_t)&PendSV_Handler, // PendSV Handler
    (uint32_t)&SysTick_Handler, // SysTick Handler
    /* 81 Maskable Interrupts IRQ*/
    (uint32_t)&WWDG_IRQHandler, /*IRQ 0*/
    (uint32_t)&EXTI16_PVD_IRQHandler, /*IRQ 1*/
    (uint32_t)&EXTI21_Tamp_Stamp_IRQHandler, /*IRQ 2*/
    (uint32_t)&EXTI22_RTC_WKUP_IRQHandler, /*IRQ 3*/
    (uint32_t)&FLASH_IRQHandler, /*IRQ 4*/
    (uint32_t)&RCC_IRQHandler, /*IRQ 5*/
    (uint32_t)&EXTI0_IRQHandler, /*IRQ 6*/
    (uint32_t)&EXTI1_IRQHandler, /*IRQ 7*/
    (uint32_t)&EXTI2_IRQHandler, /*IRQ 8*/
    (uint32_t)&EXTI3_IRQHandler, /*IRQ 9*/
    (uint32_t)&EXTI4_IRQHandler, /*IRQ 10*/
    (uint32_t)&DMA1_Stream0_IRQHandler, /*IRQ 11*/
    (uint32_t)&DMA1_Stream1_IRQHandler, /*IRQ 12*/
    (uint32_t)&DMA1_Stream2_IRQHandler, /*IRQ 13*/
    (uint32_t)&DMA1_Stream3_IRQHandler, /*IRQ 14*/
    (uint32_t)&DMA1_Stream4_IRQHandler, /*IRQ 15*/
    (uint32_t)&DMA1_Stream5_IRQHandler, /*IRQ 16*/
    (uint32_t)&DMA1_Stream6_IRQHandler, /*IRQ 17*/
    (uint32_t)&ADC_IRQHandler, /*IRQ 18*/
    0,0,0,0, /*IRQ 19-22*/
    (uint32_t)&EXTI9_5_IRQHandler, /*IRQ 23*/
    (uint32_t)&TIM1_BRK_TIM9_IRQHandler, /*IRQ 24*/
    (uint32_t)&TIM1_UP_TIM10_IRQHandler, /*IRQ 25*/
    (uint32_t)&TIM1_TRG_COM_TIM11_IRQHandler, /*IRQ 26*/
    (uint32_t)&TIM1_CC_IRQHandler, /*IRQ 27*/
    (uint32_t)&TIM2_IRQHandler, /*IRQ 28*/
    (uint32_t)&TIM3_IRQHandler, /*IRQ 29*/
    (uint32_t)&TIM4_IRQHandler, /*IRQ 30*/
    (uint32_t)&I2C1_EV_IRQHandler, /*IRQ 31*/
    (uint32_t)&I2C1_ER_IRQHandler, /*IRQ 32*/
    (uint32_t)&I2C2_EV_IRQHandler, /*IRQ 33*/
    (uint32_t)&I2C2_ER_IRQHandler, /*IRQ 34*/
    (uint32_t)&SPI1_IRQHandler, /*IRQ 35*/
    (uint32_t)&SPI2_IRQHandler, /*IRQ 36*/
    (uint32_t)&USART1_IRQHandler, /*IRQ 37*/
    (uint32_t)&USART2_IRQHandler, /*IRQ 38*/
    0, /*IRQ 39*/
    (uint32_t)&EXTI15_10_IRQHandler, /*IRQ 40*/
    (uint32_t)&EXTI17_RTC_Alarm_IRQHandler, /*IRQ 41*/
    (uint32_t)&EXTI18_OTG_FS_WKUP_IRQHandler, /*IRQ 42*/
    0, 0, 0, 0, /*43-46*/
    (uint32_t)&DMA1_Stream7_IRQHandler, /*IRQ 47*/
    0, /*IRQ 48*/
    (uint32_t)&SDIO_IRQHandler, /*IRQ 49*/
    (uint32_t)&TIM5_IRQHandler, /*IRQ 50*/
    (uint32_t)&SPI3_IRQHandler, /*IRQ 51*/

    0, 0, 0, 0, /* IRQ 52-55*/

    /* IRQ 56: DMA2 streams — STM32F411 positions */
    (uint32_t)&dma2_stream0_isr,
    (uint32_t)&dma2_stream1_isr,
    (uint32_t)&dma2_stream2_isr,
    (uint32_t)&dma2_stream3_isr,
    (uint32_t)&dma2_stream4_isr,
    0, 0, 0, 0, 0, 0, /*IRQ 61-66*/
    (uint32_t)&otg_fs_isr, /* IRQ 67: OTG FS */
    (uint32_t)&dma2_stream5_isr, /* IRQ 68 */
    (uint32_t)&dma2_stream6_isr, /* IRQ 69 */
    (uint32_t)&dma2_stream7_isr, /* IRQ 70 */
    /* IRQ 71-85*/
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

void Default_Handler(void) {
    // Handle default interrupt here
    while (1) {
        // Infinite loop to indicate interrupt occurred
    }
}
 
void Reset_Handler(void) {
    // Initialize data and bss sections here if needed
    // For example, you can copy initialized data from flash to SRAM
    // Also call stdlib initialization functions if necessary, such as __libc_init_array() for C++ static constructors
    // and zero out the bss section.

    // copy .data section to SRAM
    uint32_t size = (uint32_t)&_edata - (uint32_t)&_sdata;

    uint8_t *pDst = (uint8_t*)&_sdata;
    uint8_t *pSrc = (uint8_t*)&_la_data; /* actual flash address of .data */

    for(uint32_t i = 0; i < size; i++)
    {
        *pDst++ = *pSrc++;
    }

    //Init. the .bss section to zero in SRAM
    size = (uint32_t)&_ebss - (uint32_t)&_sbss;
    pDst = (uint8_t*)&_sbss;
    for (uint32_t i = 0; i < size; i++)
    {
        *pDst++ = 0;
    }

    //call main function
    main();
}