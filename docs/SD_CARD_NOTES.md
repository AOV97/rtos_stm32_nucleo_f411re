# SD Card Driver — Notes

## What Was Built

An SD card driver using the SPI protocol, sitting on top of the HAL SPI layer, with FatFS as the filesystem layer above it.

### Files

| File | Purpose |
|---|---|
| `ThirdParty/FatFS/ff.c` + `ff.h` | FatFS R0.16 — FAT32 filesystem library |
| `ThirdParty/FatFS/ffconf.h` | FatFS configuration for this project |
| `Drivers/sd_card/Inc/sd_card.h` | Public API: result type, `sd_init`, `sd_read_block`, `sd_write_block` |
| `Drivers/sd_card/Src/sd_card.c` | SD SPI protocol: init handshake, block read/write |
| `Drivers/sd_card/Src/diskio.c` | FatFS glue: implements the 5 disk I/O functions FatFS calls |

### How the Layers Connect

```
[ FreeRTOS Task ]
      ↓ calls f_open / f_write / f_read
[ FatFS — ff.c ]
      ↓ calls disk_read / disk_write
[ diskio.c — glue ]
      ↓ calls sd_read_block / sd_write_block
[ sd_card.c — SD protocol ]
      ↓ calls hal_spi_transfer
[ hal_spi.c — DMA SPI ]
      ↓ drives hardware
[ SPI1 + DMA2 ]
```

---

## SD Init Sequence

The init sequence forces the card into SPI mode and identifies its type. It must run at ≤400 kHz.

### Step-by-step

**1. 74+ clock pulses, CS=HIGH**
Wakes the card. It needs to see the clock before responding to anything.

**2. CMD0 — GO_IDLE_STATE**
Resets the card into SPI mode. CRC `0x95` is hardcoded — CMD0 is the only command checked before CRC can be disabled.
Expected response: `R1 = 0x01` (in idle state).

**3. CMD8 — SEND_IF_COND**
"Can you handle 3.3V?" Sends voltage range `0x01` and check pattern `0xAA`.
Expected response: `R7` — R1 byte + 4 bytes echoing the voltage and pattern back.
If the card ignores CMD8 it's an old v1 or MMC card. This driver targets v2.

**4. ACMD41 loop — SD_SEND_OP_COND**
"Are you done initialising internally?" Loops until the card reports ready.
`HCS=1` (bit 30 of argument) signals we support SDHC/SDXC.
Every application command (ACMD) must be preceded by CMD55 (APP_CMD).
Expected final response: `R1 = 0x00` (ready, no errors). Times out after 1 second.

**5. CMD58 — READ_OCR**
Reads the OCR register. Checks the **CCS bit (bit 30 of OCR)**:
- `CCS = 1` → **SDHC/SDXC** (block addressing) — all modern cards
- `CCS = 0` → **SDSC** (byte addressing) — old cards ≤2 GB

This flag is saved in `sd_is_sdhc` and used on every read and write.

**6. Reinit SPI at full speed**
After the handshake, the card is ready. SPI is reinitialised at 4 MHz (APB2 16 MHz ÷ 4). The SD spec allows up to 25 MHz.

---

## Block Read — CMD17

```
CS low
→ send CMD17 + block address
← R1 = 0x00 (command accepted)
← poll until data token 0xFE arrives
← receive 512 data bytes
← receive 2 CRC bytes (discarded)
CS high
send 1 dummy byte (recovery clock)
```

**Why the data token?** The card doesn't send data immediately — it reads from its internal flash first (takes a few microseconds). It sends `0xFF` bytes while busy, then `0xFE` to mark the start of data.

**Address translation:**
- SDHC: `addr = block` (block 5 = sector 5)
- SDSC: `addr = block * 512` (block 5 = byte 2560)

---

## Block Write — CMD24

```
CS low
→ send CMD24 + block address
← R1 = 0x00 (command accepted)
→ send 1 dummy byte (required gap)
→ send data token 0xFE
→ send 512 data bytes
→ send 2 dummy CRC bytes (0xFF 0xFF)
← receive data response byte — bits[4:1] = 0b0101 means accepted
← poll until card stops being busy (MISO returns 0xFF)
CS high
send 1 dummy byte (recovery clock)
```

**Why the busy wait?** After accepting data, the card programs it to its internal flash. This takes up to 500 ms on slow cards. MISO stays `0x00` while busy. You must wait before issuing the next command.

---

## FatFS Glue (diskio.c)

FatFS calls these five functions by name — you implement them:

| Function | What it does |
|---|---|
| `disk_initialize(0)` | Calls `sd_init()`, returns `STA_NOINIT` on failure |
| `disk_status(0)` | Returns 0 if ready, `STA_NOINIT` if not |
| `disk_read(0, buf, sector, count)` | Calls `sd_read_block()` for each sector |
| `disk_write(0, buf, sector, count)` | Calls `sd_write_block()` for each sector |
| `disk_ioctl(0, CTRL_SYNC, NULL)` | Returns `RES_OK` immediately — writes are already synchronous |

`pdrv` is the physical drive number — always `0` here (one SD card).

---

## FatFS Configuration (ffconf.h)

Key settings chosen for this project:

| Setting | Value | Why |
|---|---|---|
| `FF_FS_READONLY` | `0` | Write support needed for logging |
| `FF_USE_LFN` | `0` | No long filenames — saves ~4 KB flash |
| `FF_CODE_PAGE` | `437` | US English — smallest code page |
| `FF_FS_NORTC` | `1` | No RTC on board — timestamps fixed |
| `FF_FS_REENTRANT` | `0` | Single-task access; mutex lives in the task |
| `FF_MIN_SS / FF_MAX_SS` | `512` | SD cards always use 512-byte sectors |
| `FF_USE_STRFUNC` | `1` | Enables `f_printf` for easy log writes |
| `FF_VOLUMES` | `1` | One SD card |

---

## SPI Command Format

Every SD SPI command is 6 bytes:

```
Byte 0: 0x40 | command_index
Byte 1: argument[31:24]
Byte 2: argument[23:16]
Byte 3: argument[15:8]
Byte 4: argument[7:0]
Byte 5: (CRC7 << 1) | 0x01
```

CRC is only verified for CMD0 and CMD8. After that, CRC checking is disabled — any value works for other commands.

---

## SD Response Types

**R1** — 1 byte, returned by every command:
- `0x01` = in idle state (normal during init)
- `0x00` = ready, no errors
- `0x80` = still shifting (not a valid response yet — poll again)
- Any other value = error bits set

**R7** — R1 + 4 bytes, returned by CMD8:
- Bytes 2–3: reserved
- Byte 4: accepted voltage range (should echo `0x01`)
- Byte 5: check pattern (should echo `0xAA`)

---

## Known Limitations

- **No multi-block transfers.** `disk_read/write` loops over single-block operations. CMD18/CMD25 (multi-block) would be significantly faster for sequential access.
- **No FreeRTOS mutex.** Safe for single-task access only. A mutex around all FatFS calls is needed if multiple tasks ever use the card.
- **Write busy-wait burns CPU.** `wait_for_byte` polls without `vTaskDelay` between calls. On slow cards the busy period can be 100+ ms.
- **No error detail.** `sd_init` returns `SD_ERROR` but doesn't record which command failed. An `sd_last_error` variable would help when debugging on hardware.

---

## What Comes Next — Using FatFS

To write a log file from a FreeRTOS task:

```c
#include "ff.h"

FATFS fs;
FIL file;

f_mount(&fs, "", 1);                          /* mount the SD card */
f_open(&file, "LOG.TXT", FA_WRITE | FA_OPEN_APPEND);
f_printf(&file, "altitude: %.2f m\n", alt);
f_sync(&file);                                /* flush to card */
f_close(&file);
```

`f_mount` internally calls `disk_initialize` → `sd_init`. After that, all FatFS calls go through `diskio.c` → `sd_card.c` → `hal_spi.c`.
