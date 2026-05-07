#include "hal_spi.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include "FreeRTOSConfig.h"

/* ISRs cannot take parameters, so we keep a pointer to the active handle. */
static HAL_SPI_Handle *active_handle;

/* -------------------------------------------------------------------------
 * Clock helpers — map peripheral base addresses to their RCC enable tokens.
 * ------------------------------------------------------------------------- */

static void enable_gpio_clock(uint32_t port)
{
    switch (port) {
        case GPIOA: rcc_periph_clock_enable(RCC_GPIOA); break;
        case GPIOB: rcc_periph_clock_enable(RCC_GPIOB); break;
        case GPIOC: rcc_periph_clock_enable(RCC_GPIOC); break;
        case GPIOD: rcc_periph_clock_enable(RCC_GPIOD); break;
        default: break;
    }
}

static void enable_spi_clock(uint32_t spi)
{
    switch (spi) {
        case SPI1: rcc_periph_clock_enable(RCC_SPI1); break;
        case SPI2: rcc_periph_clock_enable(RCC_SPI2); break;
        case SPI3: rcc_periph_clock_enable(RCC_SPI3); break;
        default: break;
    }
}

static void enable_dma_clock(uint32_t dma)
{
    switch (dma) {
        case DMA1: rcc_periph_clock_enable(RCC_DMA1); break;
        case DMA2: rcc_periph_clock_enable(RCC_DMA2); break;
        default: break;
    }
}

/* -------------------------------------------------------------------------
 * hal_spi_init
 * ------------------------------------------------------------------------- */

void hal_spi_init(HAL_SPI_Handle *handle, const HAL_SPI_Config *config)
{
    active_handle = handle;

    /* 1. Clocks */
    enable_gpio_clock(handle->gpio_port);
    enable_spi_clock(handle->spi);
    enable_dma_clock(handle->dma);

    /* 2. GPIO — set SCK and MOSI as push-pull outputs, MISO as input, all AF */
    uint16_t output_pins = handle->gpio_sck | handle->gpio_mosi;
    uint16_t all_af_pins = output_pins | handle->gpio_miso;

    gpio_mode_setup(handle->gpio_port, GPIO_MODE_AF, GPIO_PUPD_NONE, all_af_pins);
    gpio_set_af(handle->gpio_port, handle->gpio_af, all_af_pins);
    gpio_set_output_options(handle->gpio_port, GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ, output_pins);

    /* 3. SPI peripheral
     *    mode encodes CPOL (bit 1) and CPHA (bit 0) — see spi_init_master docs.
     *    Software NSS: we manage CS ourselves in the driver; tell the peripheral
     *    the NSS pin is always high so it stays in master mode. */
    spi_disable(handle->spi);
    spi_init_master(handle->spi,
                    config->clock_div,
                    (config->mode & 0x02) ? SPI_CR1_CPOL : 0,
                    (config->mode & 0x01) ? SPI_CR1_CPHA : 0,
                    SPI_CR1_DFF_8BIT,
                    SPI_CR1_MSBFIRST);
    spi_enable_software_slave_management(handle->spi);
    spi_set_nss_high(handle->spi);
    spi_enable_tx_dma(handle->spi);
    spi_enable_rx_dma(handle->spi);
    spi_enable(handle->spi);

    /* 4. NVIC — set priority within FreeRTOS's syscall ceiling, then enable.
     *    Any ISR that calls a FreeRTOS API must have priority >= configMAX_SYSCALL_INTERRUPT_PRIORITY.
     *    Default NVIC priority is 0 (highest), which is above that ceiling and unsafe. */
    nvic_enable_irq(handle->nvic_rx_irq);
}

/* -------------------------------------------------------------------------
 * hal_spi_transfer
 * ------------------------------------------------------------------------- */

void hal_spi_transfer(HAL_SPI_Handle *handle, const uint8_t *tx, uint8_t *rx,
                      size_t len)
{
    /* Dummy buffers for the NULL cases: 0xFF is the SD card idle byte. */
    static uint8_t dummy_tx = 0xFF;
    static uint8_t dummy_rx;

    handle->_notify_task = xTaskGetCurrentTaskHandle();

    /* --- RX DMA (peripheral → memory) — set up FIRST --- */
    dma_stream_reset(handle->dma, handle->rx_stream);
    dma_channel_select(handle->dma, handle->rx_stream, handle->dma_channel);
    dma_set_transfer_mode(handle->dma, handle->rx_stream,
                          DMA_SxCR_DIR_PERIPHERAL_TO_MEM);
    dma_set_peripheral_address(handle->dma, handle->rx_stream,
                               (uint32_t)&SPI_DR(handle->spi));
    dma_set_memory_address(handle->dma, handle->rx_stream,
                           rx ? (uint32_t)rx : (uint32_t)&dummy_rx);
    dma_set_number_of_data(handle->dma, handle->rx_stream, (uint16_t)len);
    dma_set_memory_size(handle->dma, handle->rx_stream, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(handle->dma, handle->rx_stream, DMA_SxCR_PSIZE_8BIT);
    if (rx) dma_enable_memory_increment_mode(handle->dma, handle->rx_stream);
    else    dma_disable_memory_increment_mode(handle->dma, handle->rx_stream);
    dma_enable_transfer_complete_interrupt(handle->dma, handle->rx_stream);
    dma_enable_stream(handle->dma, handle->rx_stream);

    /* --- TX DMA (memory → peripheral) — set up SECOND to start the clock --- */
    dma_stream_reset(handle->dma, handle->tx_stream);
    dma_channel_select(handle->dma, handle->tx_stream, handle->dma_channel);
    dma_set_transfer_mode(handle->dma, handle->tx_stream,
                          DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
    dma_set_peripheral_address(handle->dma, handle->tx_stream,
                               (uint32_t)&SPI_DR(handle->spi));
    dma_set_memory_address(handle->dma, handle->tx_stream,
                           tx ? (uint32_t)tx : (uint32_t)&dummy_tx);
    dma_set_number_of_data(handle->dma, handle->tx_stream, (uint16_t)len);
    dma_set_memory_size(handle->dma, handle->tx_stream, DMA_SxCR_MSIZE_8BIT);
    dma_set_peripheral_size(handle->dma, handle->tx_stream, DMA_SxCR_PSIZE_8BIT);
    if (tx) dma_enable_memory_increment_mode(handle->dma, handle->tx_stream);
    else    dma_disable_memory_increment_mode(handle->dma, handle->tx_stream);
    dma_enable_stream(handle->dma, handle->tx_stream);

    /* Suspend this task until the RX ISR wakes us. */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

/* -------------------------------------------------------------------------
 * DMA2 Stream 0 ISR — fires when SPI1 RX DMA transfer is complete.
 *
 * portYIELD_FROM_ISR: if the task we just woke has higher priority than
 * whatever was running when the interrupt fired, switch to it immediately
 * instead of waiting for the next scheduler tick.
 * ------------------------------------------------------------------------- */

void dma2_stream0_isr(void)
{
    dma_clear_interrupt_flags(DMA2, DMA_STREAM0, DMA_TCIF);
    dma_disable_stream(DMA2, DMA_STREAM0);
    dma_disable_stream(DMA2, DMA_STREAM3);

    BaseType_t woken = pdFALSE;
    vTaskNotifyGiveFromISR(active_handle->_notify_task, &woken);
    portYIELD_FROM_ISR(woken);
}
