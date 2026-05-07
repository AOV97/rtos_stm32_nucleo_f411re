#ifndef PLATFORM_CONFIG_H_
#define PLATFORM_CONFIG_H_

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/f4/nvic.h>

/* Onboard user LED — LD2 on PA5 */
#define LED_PORT    GPIOA
#define LED_PIN     GPIO5

/* SD card — SPI1 on PA4-7 */
#define SD_SPI_PORT      GPIOA
#define SD_SPI_CS_PIN    GPIO4
#define SD_SPI_SCK_PIN   GPIO5
#define SD_SPI_MISO_PIN  GPIO6
#define SD_SPI_MOSI_PIN  GPIO7
#define SD_SPI_GPIO_AF   GPIO_AF5

/* SD card — SPI peripheral */
#define SD_SPI           SPI1

/* SD card — DMA routing (from STM32F411 reference manual, Table 27) */
#define SD_SPI_DMA          DMA2
#define SD_SPI_TX_STREAM    DMA_STREAM3
#define SD_SPI_RX_STREAM    DMA_STREAM0
#define SD_SPI_DMA_CHANNEL  DMA_SxCR_CHSEL_3
#define SD_SPI_RX_NVIC_IRQ  NVIC_DMA2_STREAM0_IRQ

#endif /* PLATFORM_CONFIG_H_ */
