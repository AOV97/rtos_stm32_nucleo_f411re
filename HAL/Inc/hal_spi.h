#ifndef HAL_SPI_H_
#define HAL_SPI_H_

#include <stddef.h>
#include <stdint.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>
#include <FreeRTOS.h>
#include <task.h>

/* Driver-supplied configuration — set once per device type. */
typedef struct {
    uint32_t clock_div;  /* SPI_CR1_BAUDRATE_FPCLK_DIV_2 .. SPI_CR1_BAUDRATE_FPCLK_DIV_256 (pre-shifted) */
    uint8_t  mode;       /* SPI mode 0-3: encodes CPOL and CPHA pair */
} HAL_SPI_Config;

/*
 * Persistent state for one SPI peripheral.
 * The driver allocates one of these and keeps it for the lifetime of the device.
 * Fill in all fields except _notify_task before calling hal_spi_init().
 */
typedef struct {
    /* SPI peripheral */
    uint32_t     spi;

    /* GPIO port and pins for SCK, MISO, MOSI */
    uint32_t     gpio_port;
    uint16_t     gpio_sck;
    uint16_t     gpio_miso;
    uint16_t     gpio_mosi;
    uint8_t      gpio_af;

    /* Chip-select pin — configured as output by hal_spi_init(), idle high */
    uint32_t     cs_port;
    uint16_t     cs_pin;

    /* DMA routing — from the chip reference manual */
    uint32_t     dma;
    uint8_t      tx_stream;
    uint8_t      rx_stream;
    uint32_t     dma_channel;   /* DMA_SxCR_CHSEL_x constant */
    uint8_t      nvic_rx_irq;   /* NVIC IRQ number for the RX DMA stream */

    /* Internal — written by hal_spi_transfer(), do not initialize */
    TaskHandle_t _notify_task;
} HAL_SPI_Handle;

/*
 * Set up clocks, GPIO alternate functions, the SPI peripheral, DMA, and the
 * CS pin (output, idle high). Call before any transfers.
 */
void hal_spi_init(HAL_SPI_Handle *handle, const HAL_SPI_Config *config);

/* Assert / deassert chip select (active low). */
void hal_spi_cs_assert(HAL_SPI_Handle *handle);
void hal_spi_cs_deassert(HAL_SPI_Handle *handle);

/*
 * Blocking full-duplex transfer: sends `len` bytes from `tx` while receiving
 * into `rx`. Pass NULL for tx to send 0xFF (SD card idle byte). Pass NULL for
 * rx to discard received data. Suspends the calling FreeRTOS task until done.
 */
void hal_spi_transfer(HAL_SPI_Handle *handle, const uint8_t *tx, uint8_t *rx,
                      size_t len);

#endif /* HAL_SPI_H_ */
