#include "tasks/sd_task.h"
#include "ff.h"
#include "FreeRTOS.h"
#include "task.h"

void sd_task(void *params)
{
    (void)params;

    /* Declared static so they live in .bss RAM, not on the task stack.
     * FATFS and FIL are each ~560 bytes — too large for a small stack. */
    static FATFS fs;
    static FIL   file;
    FRESULT res;

    /* Mount volume 0 (the SD card). The '1' means mount immediately —
     * this is where disk_initialize → sd_init runs. */
    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        for (;;) vTaskDelay(pdMS_TO_TICKS(500));
    }

    /* Open HELLO.TXT for writing. FA_OPEN_APPEND creates the file if it
     * does not exist and positions the write pointer at the end if it does,
     * so each boot adds a new line rather than overwriting the previous one. */
    res = f_open(&file, "HELLO.TXT", FA_WRITE | FA_OPEN_APPEND);
    if (res != FR_OK) {
        f_unmount("");
        for (;;) vTaskDelay(pdMS_TO_TICKS(500));
    }

    f_printf(&file, "Hello World\n");

    /* f_sync flushes the write buffer to the card without closing the file.
     * f_close does the same flush and then releases the file object. */
    f_sync(&file);
    f_close(&file);
    f_unmount("");

    /* One-shot task — suspend rather than spin or delete. */
    vTaskSuspend(NULL);
}
