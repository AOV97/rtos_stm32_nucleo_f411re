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

    /* Actual flash address where .data is stored — may differ from _etext
     * if the linker inserts ARM interworking sections between .text and .data */
    _la_data = LOADADDR(.data);

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

}
