#include "ff.h"
#include "diskio.h"
#include "sd_card.h"

/*
 * FatFS disk I/O glue — connects FatFS to the sd_card driver.
 *
 * pdrv is the physical drive number. We only have one drive (the SD card),
 * so every function checks pdrv == 0 and rejects anything else.
 */

static DSTATUS disk_state = STA_NOINIT;

DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv != 0) return STA_NOINIT;
    if (sd_init() == SD_OK) {
        disk_state = 0;
    }
    return disk_state;
}

DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != 0) return STA_NOINIT;
    return disk_state;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
    if (pdrv != 0 || disk_state & STA_NOINIT) return RES_NOTRDY;

    for (UINT i = 0; i < count; i++) {
        if (sd_read_block(sector + i, buff + (i * 512)) != SD_OK) {
            return RES_ERROR;
        }
    }
    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
    if (pdrv != 0 || disk_state & STA_NOINIT) return RES_NOTRDY;

    for (UINT i = 0; i < count; i++) {
        if (sd_write_block(sector + i, (const uint8_t *)(buff + (i * 512))) != SD_OK) {
            return RES_ERROR;
        }
    }
    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    if (pdrv != 0 || disk_state & STA_NOINIT) return RES_NOTRDY;
    (void)buff;

    switch (cmd) {
        case CTRL_SYNC:
            /* SD writes are synchronous — sd_write_block only returns after the
             * card finishes programming, so there is nothing to flush here. */
            return RES_OK;
        default:
            return RES_PARERR;
    }
}
