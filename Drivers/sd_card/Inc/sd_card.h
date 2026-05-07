#ifndef SD_CARD_H_
#define SD_CARD_H_

#include <stdint.h>

typedef enum {
    SD_OK = 0,
    SD_ERROR,
    SD_TIMEOUT,
} SD_Result;

/*
 * Initialise the SD card over SPI. Must be called from a FreeRTOS task
 * (not before the scheduler starts) because it uses DMA transfers internally.
 * Runs the full SD SPI init handshake: CMD0 → CMD8 → ACMD41 → CMD58.
 */
SD_Result sd_init(void);

/* Read one 512-byte block at the given block address into buf. */
SD_Result sd_read_block(uint32_t block, uint8_t *buf);

/* Write one 512-byte block from buf to the given block address. */
SD_Result sd_write_block(uint32_t block, const uint8_t *buf);

#endif /* SD_CARD_H_ */
