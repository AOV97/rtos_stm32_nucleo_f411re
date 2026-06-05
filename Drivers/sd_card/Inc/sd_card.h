#ifndef SD_CARD_H_
#define SD_CARD_H_

#include <stdint.h>

typedef enum {
    SD_OK = 0,
    SD_ERROR,
    SD_TIMEOUT,
} SD_Result;

/**
 * @brief Initializes the SD card over SPI using the standard SD SPI handshake.
 *
 * Executes the full initialization sequence: CMD0 (idle/reset) → CMD8 (verify
 * voltage range) → ACMD41 (activate initialization) → CMD58 (read OCR). Must
 * be called from a FreeRTOS task after the scheduler has started because it
 * uses DMA-backed SPI transfers internally (blocks on task notification).
 *
 * @return SD_OK on success. SD_TIMEOUT if the card does not respond within the
 *         expected number of retries. SD_ERROR for any other protocol failure.
 */
SD_Result sd_init(void);

/**
 * @brief Reads one 512-byte block from the SD card into a caller-supplied buffer.
 *
 * Sends CMD17 (READ_SINGLE_BLOCK) to the given block address and waits for the
 * data token, then transfers 512 bytes into buf via DMA. The block address is
 * a logical sector number (not a byte offset); each sector is 512 bytes.
 *
 * @param block  Logical block address (sector number) to read from.
 * @param buf    Pointer to a buffer of at least 512 bytes to receive the data.
 *
 * @return SD_OK on success, SD_TIMEOUT if the card stalls, SD_ERROR on
 *         protocol or CRC failure.
 */
SD_Result sd_read_block(uint32_t block, uint8_t *buf);

/**
 * @brief Writes one 512-byte block from a caller-supplied buffer to the SD card.
 *
 * Sends CMD24 (WRITE_BLOCK) to the given block address, transmits the data
 * token and 512 bytes from buf via DMA, then waits for the card to signal
 * write completion.
 *
 * @param block  Logical block address (sector number) to write to.
 * @param buf    Pointer to a buffer of at least 512 bytes to write.
 *
 * @return SD_OK on success, SD_TIMEOUT if the card stalls during programming,
 *         SD_ERROR on protocol failure.
 */
SD_Result sd_write_block(uint32_t block, const uint8_t *buf);

#endif /* SD_CARD_H_ */
