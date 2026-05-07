#include "sd_card.h"
#include "hal_spi.h"
#include "platform_config.h"
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>

/* -------------------------------------------------------------------------
 * Hardware state
 * ------------------------------------------------------------------------- */

static HAL_SPI_Handle sd_spi = {
    .spi         = SD_SPI,
    .gpio_port   = SD_SPI_PORT,
    .gpio_sck    = SD_SPI_SCK_PIN,
    .gpio_miso   = SD_SPI_MISO_PIN,
    .gpio_mosi   = SD_SPI_MOSI_PIN,
    .gpio_af     = SD_SPI_GPIO_AF,
    .cs_port     = SD_SPI_PORT,
    .cs_pin      = SD_SPI_CS_PIN,
    .dma         = SD_SPI_DMA,
    .tx_stream   = SD_SPI_TX_STREAM,
    .rx_stream   = SD_SPI_RX_STREAM,
    .dma_channel = SD_SPI_DMA_CHANNEL,
    .nvic_rx_irq = SD_SPI_RX_NVIC_IRQ,
};

static bool sd_initialized = false;
static bool sd_is_sdhc = false;     /* true = block addressing, false = byte addressing */

/* One dummy byte — SD cards need a clock between CS edges to recover */
static void spi_dummy(void)
{
    uint8_t b = 0xFF;
    hal_spi_transfer(&sd_spi, &b, NULL, 1);
}

/* -------------------------------------------------------------------------
 * Command framing
 *
 * Every SD SPI command is 6 bytes:
 *   [0x40|cmd] [arg3] [arg2] [arg1] [arg0] [crc7|0x01]
 *
 * CRC is only verified for CMD0 and CMD8; pass 0xFF for all others.
 * ------------------------------------------------------------------------- */

static void send_cmd(uint8_t cmd, uint32_t arg, uint8_t crc)
{
    uint8_t buf[6] = {
        0x40 | cmd,
        (arg >> 24) & 0xFF,
        (arg >> 16) & 0xFF,
        (arg >>  8) & 0xFF,
        (arg >>  0) & 0xFF,
        (crc << 1) | 0x01,
    };
    hal_spi_transfer(&sd_spi, buf, NULL, 6);
}

/* -------------------------------------------------------------------------
 * Response parsing
 *
 * After a command the card drives MISO high for up to 8 bytes before
 * returning a response. We poll until the high bit is clear (valid R1).
 * ------------------------------------------------------------------------- */

static uint8_t read_r1(void)
{
    uint8_t r1 = 0xFF;
    for (int i = 0; i < 8; i++) {
        hal_spi_transfer(&sd_spi, NULL, &r1, 1);
        if (!(r1 & 0x80)) break;    /* bit 7 clear = valid R1 byte */
    }
    return r1;
}

/* Wait for a specific byte from the card with timeout.
 * Used to wait for the data token (0xFE) before a read. */
static SD_Result wait_for_byte(uint8_t target, uint32_t timeout_ms)
{
    uint8_t b = 0xFF;
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(timeout_ms);
    do {
        hal_spi_transfer(&sd_spi, NULL, &b, 1);
        if (b == target) return SD_OK;
    } while (xTaskGetTickCount() < deadline);
    return SD_TIMEOUT;
}

/* -------------------------------------------------------------------------
 * sd_init
 *
 * Full SPI mode initialisation sequence per the SD Physical Layer Spec:
 *   CMD0 → CMD8 → ACMD41 (with CMD55 prefix) → CMD58
 * ------------------------------------------------------------------------- */

SD_Result sd_init(void)
{
    /* --- Phase 1: slow clock for init (≤400 kHz) ---
     * APB2 = 16 MHz (HSI), DIV256 = 62.5 kHz */
    HAL_SPI_Config slow = { .clock_div = SPI_CR1_BAUDRATE_FPCLK_DIV_256, .mode = 0 };
    hal_spi_init(&sd_spi, &slow);   /* configures SPI and CS (idle high) */

    /* 74+ clock pulses with CS=HIGH to wake the card */
    uint8_t wake[10];
    memset(wake, 0xFF, sizeof(wake));
    hal_spi_cs_deassert(&sd_spi);
    hal_spi_transfer(&sd_spi, wake, NULL, sizeof(wake));

    /* CMD0: reset into SPI mode. CRC 0x4A (pre-computed for CMD0+arg=0) */
    hal_spi_cs_assert(&sd_spi);
    send_cmd(0, 0x00000000, 0x4A);
    uint8_t r1 = read_r1();
    hal_spi_cs_deassert(&sd_spi);
    spi_dummy();
    if (r1 != 0x01) return SD_ERROR;

    /* CMD8: check voltage range and confirm SD v2.
     * Argument: VHS=1 (2.7–3.6 V) | check pattern=0xAA
     * CRC 0x43 (pre-computed for CMD8 + this argument) */
    hal_spi_cs_assert(&sd_spi);
    send_cmd(8, 0x000001AA, 0x43);
    r1 = read_r1();
    uint8_t r7[4];
    hal_spi_transfer(&sd_spi, NULL, r7, 4);
    hal_spi_cs_deassert(&sd_spi);
    spi_dummy();
    if (r1 != 0x01)             return SD_ERROR;
    if (r7[2] != 0x01)          return SD_ERROR;    /* voltage range rejected */
    if (r7[3] != 0xAA)          return SD_ERROR;    /* check pattern mismatch */

    /* ACMD41: wait for card to finish internal init.
     * ACMD = application command, always preceded by CMD55.
     * HCS=1 (bit 30) signals we support SDHC/SDXC.
     * Loop up to 1 second — most cards finish in <100 ms. */
    TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(1000);
    do {
        hal_spi_cs_assert(&sd_spi);
        send_cmd(55, 0x00000000, 0xFF);     /* CMD55: next cmd is application cmd */
        r1 = read_r1();
        hal_spi_cs_deassert(&sd_spi);
        spi_dummy();

        hal_spi_cs_assert(&sd_spi);
        send_cmd(41, 0x40000000, 0xFF);     /* ACMD41: init, HCS=1 */
        r1 = read_r1();
        hal_spi_cs_deassert(&sd_spi);
        spi_dummy();

        if (r1 == 0x00) break;              /* 0x00 = ready */
        vTaskDelay(pdMS_TO_TICKS(1));
    } while (xTaskGetTickCount() < deadline);

    if (r1 != 0x00) return SD_TIMEOUT;

    /* CMD58: read OCR register. Check the CCS bit (bit 30) to detect SDHC.
     * CCS=1 → block addressing (modern cards).
     * CCS=0 → byte addressing (old <2 GB SDSC cards). */
    hal_spi_cs_assert(&sd_spi);
    send_cmd(58, 0x00000000, 0xFF);
    r1 = read_r1();
    uint8_t ocr[4];
    hal_spi_transfer(&sd_spi, NULL, ocr, 4);
    hal_spi_cs_deassert(&sd_spi);
    spi_dummy();
    if (r1 != 0x00) return SD_ERROR;
    sd_is_sdhc = (ocr[0] & 0x40) != 0;

    /* --- Phase 2: switch to full speed ---
     * APB2 = 16 MHz, DIV4 = 4 MHz (safe for most SD cards; spec allows 25 MHz) */
    HAL_SPI_Config fast = { .clock_div = SPI_CR1_BAUDRATE_FPCLK_DIV_4, .mode = 0 };
    hal_spi_init(&sd_spi, &fast);

    sd_initialized = true;
    return SD_OK;
}

/* -------------------------------------------------------------------------
 * sd_read_block
 *
 * CMD17 (READ_SINGLE_BLOCK):
 *   1. Send CMD17 with block address (SDHC) or byte address (SDSC)
 *   2. Wait for R1 = 0x00
 *   3. Wait for data token 0xFE
 *   4. Read 512 data bytes
 *   5. Read and discard 2-byte CRC
 * ------------------------------------------------------------------------- */

SD_Result sd_read_block(uint32_t block, uint8_t *buf)
{
    if (!sd_initialized) return SD_ERROR;

    uint32_t addr = sd_is_sdhc ? block : block * 512;

    hal_spi_cs_assert(&sd_spi);
    send_cmd(17, addr, 0xFF);
    uint8_t r1 = read_r1();
    if (r1 != 0x00) { hal_spi_cs_deassert(&sd_spi); spi_dummy(); return SD_ERROR; }

    /* Data token 0xFE arrives before the actual data — wait up to 200 ms */
    if (wait_for_byte(0xFE, 200) != SD_OK) {
        hal_spi_cs_deassert(&sd_spi);
        spi_dummy();
        return SD_TIMEOUT;
    }

    hal_spi_transfer(&sd_spi, NULL, buf, 512);

    uint8_t crc[2];
    hal_spi_transfer(&sd_spi, NULL, crc, 2);    /* discard CRC */

    hal_spi_cs_deassert(&sd_spi);
    spi_dummy();
    return SD_OK;
}

/* -------------------------------------------------------------------------
 * sd_write_block
 *
 * CMD24 (WRITE_BLOCK):
 *   1. Send CMD24 with block/byte address
 *   2. Wait for R1 = 0x00
 *   3. Send 1 dummy byte, then data token 0xFE, then 512 data bytes
 *   4. Send 2 dummy CRC bytes
 *   5. Read data response: low nibble 0x5 = accepted
 *   6. Wait while card is busy (MISO = 0x00)
 * ------------------------------------------------------------------------- */

SD_Result sd_write_block(uint32_t block, const uint8_t *buf)
{
    if (!sd_initialized) return SD_ERROR;

    uint32_t addr = sd_is_sdhc ? block : block * 512;

    hal_spi_cs_assert(&sd_spi);
    send_cmd(24, addr, 0xFF);
    uint8_t r1 = read_r1();
    if (r1 != 0x00) { hal_spi_cs_deassert(&sd_spi); spi_dummy(); return SD_ERROR; }

    spi_dummy();                                /* 1 byte gap required before data */

    uint8_t token = 0xFE;
    hal_spi_transfer(&sd_spi, &token, NULL, 1);
    hal_spi_transfer(&sd_spi, buf, NULL, 512);

    uint8_t crc[2] = { 0xFF, 0xFF };
    hal_spi_transfer(&sd_spi, crc, NULL, 2);    /* dummy CRC */

    /* Data response: bits [4:1] = 0b0101 means accepted */
    uint8_t resp;
    hal_spi_transfer(&sd_spi, NULL, &resp, 1);
    if ((resp & 0x1F) != 0x05) { hal_spi_cs_deassert(&sd_spi); spi_dummy(); return SD_ERROR; }

    /* Wait while card programs the flash — up to 500 ms */
    if (wait_for_byte(0xFF, 500) != SD_OK) {
        hal_spi_cs_deassert(&sd_spi);
        spi_dummy();
        return SD_TIMEOUT;
    }

    hal_spi_cs_deassert(&sd_spi);
    spi_dummy();
    return SD_OK;
}
