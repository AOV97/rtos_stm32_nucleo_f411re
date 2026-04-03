#include <stdint.h>

#define SRAM_START 0x20000000u
#define SRAM_SIZE (128u * 1024u) // 128 KB
#define SRAM_END (SRAM_START) + (SRAM_SIZE)

#define STACK_START SRAM_END

extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

//prototype of main 
int main(void);

//Function prototype for each IRQ handler

/**
 * @brief System Exception Handlers
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

uint32_t vector[]  __attribute__((section(".isr_vector"))) = {
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
    (uint32_t)&WWDG_IRQHandler,
    (uint32_t)&EXTI16_PVD_IRQHandler,
    (uint32_t)&EXTI21_Tamp_Stamp_IRQHandler,
    (uint32_t)&EXTI22_RTC_WKUP_IRQHandler,
    (uint32_t)&FLASH_IRQHandler,
    (uint32_t)&RCC_IRQHandler,
    (uint32_t)&EXTI0_IRQHandler,
    (uint32_t)&EXTI1_IRQHandler,
    (uint32_t)&EXTI2_IRQHandler,
    (uint32_t)&EXTI3_IRQHandler,
    (uint32_t)&EXTI4_IRQHandler,
    (uint32_t)&DMA1_Stream0_IRQHandler,
    (uint32_t)&DMA1_Stream1_IRQHandler,
    (uint32_t)&DMA1_Stream2_IRQHandler,
    (uint32_t)&DMA1_Stream3_IRQHandler,
    (uint32_t)&DMA1_Stream4_IRQHandler,
    (uint32_t)&DMA1_Stream5_IRQHandler,
    (uint32_t)&DMA1_Stream6_IRQHandler,
    (uint32_t)&ADC_IRQHandler,
    (uint32_t)&EXTI9_5_IRQHandler,
    (uint32_t)&TIM1_BRK_TIM9_IRQHandler,
    (uint32_t)&TIM1_UP_TIM10_IRQHandler,
    (uint32_t)&TIM1_TRG_COM_TIM11_IRQHandler,
    (uint32_t)&TIM1_CC_IRQHandler,
    (uint32_t)&TIM2_IRQHandler,
    (uint32_t)&TIM3_IRQHandler,
    (uint32_t)&TIM4_IRQHandler,
    (uint32_t)&I2C1_EV_IRQHandler,
    (uint32_t)&I2C1_ER_IRQHandler,
    (uint32_t)&I2C2_EV_IRQHandler,
    (uint32_t)&I2C2_ER_IRQHandler,
    (uint32_t)&SPI1_IRQHandler,
    (uint32_t)&SPI2_IRQHandler,
    (uint32_t)&USART1_IRQHandler,
    (uint32_t)&USART2_IRQHandler,
    (uint32_t)&EXTI15_10_IRQHandler,
    (uint32_t)&EXTI17_RTC_Alarm_IRQHandler,
    (uint32_t)&EXTI18_OTG_FS_WKUP_IRQHandler,
    (uint32_t)&DMA1_Stream7_IRQHandler,
    (uint32_t)&SDIO_IRQHandler,
    (uint32_t)&TIM5_IRQHandler,
    (uint32_t)&SPI3_IRQHandler,

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
    uint8_t *pSrc = (uint8_t*)&_etext; //flash

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