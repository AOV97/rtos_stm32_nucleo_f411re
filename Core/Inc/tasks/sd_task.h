#ifndef SD_TASK_H_
#define SD_TASK_H_

/**
 * @brief FreeRTOS task that exercises the SD card driver.
 *
 * Calls sd_init() to perform the full SPI SD card handshake, then performs a
 * test write followed by a read-back on block 0 to verify the SPI/DMA path.
 * Results are reported through the logger. Must run after the scheduler starts
 * because sd_init() uses DMA transfers internally (requires task context for
 * task notification blocking).
 *
 * @param params  Unused. Pass NULL when creating with xTaskCreate.
 */
void sd_task(void *params);

#endif /* SD_TASK_H_ */
