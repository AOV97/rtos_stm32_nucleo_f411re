ENTRY(Reset_Handler)

MEMORY
{
    FLASH(rx) : ORIGIN = 0x08000000, LENGTH = 512K
    SRAM(rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

SECTIONS
{
    /* ---- Code + read-only data in FLASH ---- */
    .text :
    {
        *(.isr_vector)
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* ---- Initialised globals: stored in FLASH, copied to SRAM at boot ---- */
    .data :
    {
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM AT> FLASH

    /* ---- Zero-initialised globals in SRAM ---- */
    .bss :
    {
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM

    /* ---- FreeRTOS heap (heap_4.c uses this region via ucHeap[]) ---- */
    /* configTOTAL_HEAP_SIZE in FreeRTOSConfig.h must match the size here  */
    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        _heap_start = .;
        . = . + 0x6000;   /* 24 KB */
        _heap_end = .;
    } > SRAM
}
