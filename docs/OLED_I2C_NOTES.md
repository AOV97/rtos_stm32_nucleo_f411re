# OLED Display Integration (SSD1306 over I2C)

## Hardware

| Item | Detail |
|---|---|
| Display | SSD1306 128×64 OLED |
| Interface | I2C (2-wire) |
| I2C peripheral | I2C1 |
| SCL | PB8 (CN10 pin 3, AF4) |
| SDA | PB9 (CN10 pin 5, AF4) |
| I2C address | 0x3C (7-bit) |
| Supply | 3.3V |
| Speed | 100 kHz standard mode |

**Pull-up resistors:** SCL and SDA both require a pull-up to 3.3V. Most SSD1306 breakout boards include 4.7 kΩ resistors on-board. If yours does not, add them externally.

---

## Software Architecture

This integration follows the project's three-layer design. No layer reaches into the layer below it (see `ARCHITECTURE.md`).

```
Core/Src/tasks/oled_task.c        ← application task (Layer 3)
        |
        v
Drivers/oled/Src/ssd1306.c        ← device driver (Layer 2)
        |
        v
HAL/Src/hal_i2c.c                 ← hardware abstraction (Layer 1)
        |
        v
libopencm3 I2C API                ← peripheral register access
```

Pin and peripheral constants live exclusively in `BSP/stm32f411_nucleo/platform_config.h`. `BSP/Src/bsp.c` consumes those constants and exposes `bsp_oled_i2c_init()` so that Core/ never includes `platform_config.h` directly. Only `bsp.c` and `platform_config.h` would need to change for a different board.

---

## Files Added

| File | Description |
|---|---|
| `HAL/Inc/hal_i2c.h` | I2C HAL interface |
| `HAL/Src/hal_i2c.c` | libopencm3 I2C implementation |
| `Drivers/oled/Inc/ssd1306.h` | SSD1306 driver interface |
| `Drivers/oled/Src/ssd1306.c` | SSD1306 driver + 5×8 ASCII font |
| `Drivers/oled/CMakeLists.txt` | Build target for `oled` static library |
| `Core/Inc/tasks/oled_task.h` | FreeRTOS task declaration |
| `Core/Src/tasks/oled_task.c` | FreeRTOS task implementation |

### Files Modified

| File | Change |
|---|---|
| `BSP/stm32f411_nucleo/platform_config.h` | Added `OLED_I2C*` and `OLED_I2C_ADDR` defines |
| `BSP/Inc/bsp.h` | Added `#include "hal_i2c.h"`, declared `bsp_oled_i2c_init()`, exported `BSP_OLED_I2C_ADDR` |
| `BSP/Src/bsp.c` | Implemented `bsp_oled_i2c_init()` — owns all platform pin constants, calls `hal_i2c_init()` |
| `BSP/CMakeLists.txt` | Added `HAL/Inc` to BSP include path |
| `HAL/CMakeLists.txt` | Added `hal_i2c.c` to `hal` library |
| `Drivers/CMakeLists.txt` | Added `add_subdirectory(oled)` |
| `Core/CMakeLists.txt` | Added `oled_task.c`; linked `oled` library |
| `Core/Src/main.c` | Created `oled_task` via `xTaskCreate` |

---

## HAL Layer — `hal_i2c`

### API

```c
void hal_i2c_init(HAL_I2C_Handle *h);
bool hal_i2c_write(HAL_I2C_Handle *h, uint8_t addr7,
                   const uint8_t *buf, size_t len);
```

`hal_i2c_init` enables GPIO clocks, sets both pins to open-drain alternate function (AF4), then configures the I2C peripheral:

| Register field | Value | Reason |
|---|---|---|
| `FREQ` | 16 | APB1 clock = 16 MHz (HSI, no PLL) |
| `CCR` | 80 | 16 MHz ÷ (2 × 100 kHz) |
| `TRISE` | 17 | (1000 ns × 16 MHz) + 1 |

`hal_i2c_write` performs a blocking master write using register polling. Every wait step (bus idle, SB, ADDR, TxE, BTF) has a countdown timeout and returns `false` on failure or NACK, so the driver can handle errors without hanging.

The GPIO lines are configured with `GPIO_OTYPE_OD` (open-drain). **Do not use push-pull for I2C** — both devices driving the line in opposite directions causes bus contention and can damage hardware.

---

## Driver Layer — `ssd1306`

### Framebuffer layout

The SSD1306 GDDRAM is organised as 8 pages × 128 columns. Each byte encodes 8 vertical pixels, with bit 0 at the top of the page.

```
Page 0  — rows 0–7   (top)
Page 1  — rows 8–15
...
Page 7  — rows 56–63 (bottom)
```

The driver holds a 1 KB framebuffer (`uint8_t fb[8][128]`) in SRAM. All drawing calls modify the framebuffer only. `ssd1306_flush()` pushes the buffer to the display.

### Initialization sequence

The init sequence sent over I2C configures the display for 3.3V single-supply operation (charge pump on), horizontal addressing mode, and correct segment/COM orientation for the common 128×64 module:

```
0xAE        display off
0xD5, 0x80  clock divide ratio
0xA8, 0x3F  multiplex ratio = 63 (64 rows)
0xD3, 0x00  display offset = 0
0x40        start line = 0
0x8D, 0x14  charge pump on
0x20, 0x00  horizontal addressing mode
0xA1        segment remap (col 127 → SEG0)
0xC8        COM scan remapped (top → bottom)
0xDA, 0x12  COM pins: alternative
0x81, 0xCF  contrast
0xD9, 0xF1  pre-charge period
0xDB, 0x40  VCOMH deselect
0xA4        follow RAM (not force-on)
0xA6        normal display (not inverted)
0xAF        display on
```

### SSD1306 I2C protocol

Every I2C transaction to the SSD1306 starts with a **control byte**:

| Control byte | Meaning |
|---|---|
| `0x00` | All following bytes are commands |
| `0x40` | All following bytes are pixel data (GDDRAM) |

`ssd1306_flush()` first sends a command packet resetting the column/page address pointers to (0, 0), then sends 8 data packets of 129 bytes each (1 control byte + 128 pixel bytes per page).

### Font

A standard 5×8 bitmap font covers all 95 printable ASCII characters (0x20–0x7E). Each character is 5 bytes wide (one byte per column); `ssd1306_draw_char` writes those 5 bytes into the framebuffer page, leaving the 6th column clear as a spacer. This gives a character cell of 6×8 pixels, fitting up to 21 characters per page row.

---

## Application Layer — `oled_task`

The task delegates I2C setup to the BSP (`bsp_oled_i2c_init()`), which fills the `HAL_I2C_Handle` with board-specific pin constants and calls `hal_i2c_init()`. The task then initialises the SSD1306 handle and loops every 500 ms:

1. Clear the framebuffer.
2. Draw static strings on pages 0, 2, and 6.
3. Draw a live tick counter on page 4.
4. Call `ssd1306_flush()` to push to the display.
5. `vTaskDelay(pdMS_TO_TICKS(500))` — yields to the scheduler.

### Display output

```
Page 0:  STM32F411 + RTOS
Page 2:  I2C SSD1306 128x64
Page 4:  Tick: <n>          ← increments every 500 ms
Page 6:  Page 6 ready
```

---

## Adding More I2C Devices

The bus architecture note from `ARCHITECTURE.md` applies: **mutexes for shared buses belong in the driver, not the HAL.** If a second I2C device is added (e.g. BMP388 on the same I2C1 bus):

1. Create a new driver under `Drivers/<device>/`.
2. Pass the same `HAL_I2C_Handle` pointer to both drivers.
3. Add an `SemaphoreHandle_t` I2C bus mutex and take/give it around every `hal_i2c_write` call in each driver.

The HAL itself stays unchanged.
