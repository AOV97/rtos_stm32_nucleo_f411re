# HAL SPI — Notes

## What Was Built

A generic SPI HAL for the STM32F411RE using libopencm3 and FreeRTOS DMA transfers.

### Files

| File | Purpose |
|---|---|
| `HAL/Inc/hal_spi.h` | Public API: config struct, handle struct, function declarations |
| `HAL/Src/hal_spi.c` | Implementation: clocks, GPIO, SPI init, DMA transfer, ISR |
| `BSP/stm32f411_nucleo/platform_config.h` | Pin assignments, SPI peripheral, DMA routing |
| `HAL/CMakeLists.txt` | Builds `hal` static library |

---

## How It Works

### The Two Structs

**`HAL_SPI_Config`** — what the driver provides to describe its device:
- `clock_div` — SPI bus speed (e.g. `SPI_CR1_BR_FPCLK_DIV256` for ~400 kHz, used during SD card init)
- `mode` — SPI mode 0–3, encodes CPOL and CPHA (SD card uses mode 0)

**`HAL_SPI_Handle`** — persistent state, one per device, lives for the lifetime of the driver:
- SPI peripheral base address (`SPI1`)
- GPIO port and pin numbers for SCK, MISO, MOSI
- GPIO alternate function number (`GPIO_AF5` for SPI1 on PA5–PA7)
- DMA controller, TX stream, RX stream, channel
- NVIC IRQ number for the RX DMA stream
- `_notify_task` — set internally by `hal_spi_transfer`, do not initialize

The driver fills in the handle fields from `platform_config.h` defines and keeps it alive. It passes it into every HAL call.

### hal_spi_init

1. Enables RCC clocks for the GPIO port, SPI peripheral, and DMA controller
2. Configures SCK and MOSI as push-pull outputs, MISO as input — all set to alternate function `gpio_af`
3. Calls `spi_init_master` with the clock divider and CPOL/CPHA derived from `config->mode`
4. Enables software NSS management (the driver owns the CS pin, not the SPI peripheral)
5. Enables SPI TX and RX DMA requests
6. Enables the SPI peripheral
7. Enables the NVIC interrupt for the RX DMA stream

### hal_spi_transfer

Full-duplex: sends `len` bytes while simultaneously receiving `len` bytes.

- Pass `NULL` for `tx` to send `0xFF` (SD card idle byte) without a real buffer
- Pass `NULL` for `rx` to discard received data
- **RX DMA is set up first** — TX DMA starting the stream kicks off the SPI clock; if RX isn't armed first, the first incoming byte is lost
- After both DMA streams are enabled, the function calls `ulTaskNotifyTake` — the calling FreeRTOS task sleeps and yields the CPU to other tasks while DMA runs in hardware
- When the RX DMA ISR fires, it calls `vTaskNotifyGiveFromISR` to wake the sleeping task

### dma2_stream0_isr (RX complete ISR)

- Clears the transfer-complete interrupt flag
- Disables both DMA streams (RX stream 0 and TX stream 3)
- Calls `vTaskNotifyGiveFromISR` to unblock the task waiting in `hal_spi_transfer`
- Calls `portYIELD_FROM_ISR` — if the woken task has higher priority than whatever was interrupted, the scheduler switches immediately rather than waiting for the next tick

---

## Why DMA Instead of Polling

Polling burns the CPU in a tight loop checking a "transfer done" flag. The processor cannot run any other FreeRTOS task while waiting. DMA offloads the byte-by-byte copy work to a dedicated hardware engine. The CPU hands DMA a buffer and goes to sleep. For a 512-byte SD card sector at 25 MHz, that's ~160 µs of free CPU time per read.

## Why Wait on RX Complete, Not TX Complete

TX DMA finishes when it pushes the last byte into the SPI shift register — but that byte still has to be clocked out. RX DMA completes only after the last bit has actually arrived in memory. RX complete is the true "transfer done" signal.

---

## DMA Routing (from STM32F411 Reference Manual, Table 27)

SPI1 peripheral is hardwired to specific DMA streams — these cannot be changed:

| Direction | DMA | Stream | Channel |
|---|---|---|---|
| SPI1 RX | DMA2 | Stream 0 | Channel 3 |
| SPI1 TX | DMA2 | Stream 3 | Channel 3 |

The RX stream's NVIC IRQ is `NVIC_DMA2_STREAM0_IRQ` (IRQ 56).

---

## SPI Mode Table

| Mode | CPOL | CPHA | Clock idles | Sample on |
|---|---|---|---|---|
| 0 | 0 | 0 | Low | Rising edge |
| 1 | 0 | 1 | Low | Falling edge |
| 2 | 1 | 0 | High | Falling edge |
| 3 | 1 | 1 | High | Rising edge |

SD cards use **Mode 0**.

---

## Known Limitations

- `hal_spi_transfer` uses `portMAX_DELAY` — it will wait forever if DMA never completes. A production driver should pass a finite timeout and handle the error.
- The `static HAL_SPI_Handle *active_handle` global means two simultaneous SPI transfers on different buses would corrupt each other. Fine for single-bus use; would need per-stream dispatch for multi-bus.
- No DMA error handling. The STM32 DMA has a separate transfer-error interrupt (`DMA_TEIF`). Not implemented here.
- `hal_spi_transfer` returns `void`. A real driver needs an error return so the application can recover from failures.

---

## What Comes Next — SD Card Driver

The SD driver (`Drivers/sd_card/`) will:
1. Declare a `HAL_SPI_Handle` filled with the `SD_SPI_*` defines from `platform_config.h`
2. Configure the CS pin (PA4) as a GPIO output — the driver owns CS, not the HAL
3. Call `hal_spi_init` twice: once slow (≤400 kHz, mode 0) for the SD init handshake, then fast for data transfers
4. Assert CS low, call `hal_spi_transfer`, then release CS high for each transaction
