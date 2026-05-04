# STM32F411RE Bare-Metal FreeRTOS

Bare-metal FreeRTOS application for the STM32F411RE Nucleo-64 board.
Built without an IDE using `arm-none-eabi-gcc`, CMake, a hand-written linker script,
and libopencm3 as the peripheral driver library.

**Current demo — circular buffer producer/consumer:**
A producer task writes ON/OFF commands every 100 ms into an 8-slot circular buffer.
A consumer task reads one command every 500 ms and drives LD2 (PA5) accordingly.
The buffer fills after ~800 ms, after which the producer blocks until the consumer
frees a slot. LD2 then blinks at the consumer rate (500 ms) instead of the producer rate.

---

## Requirements

```bash
sudo apt install gcc-arm-none-eabi cmake make openocd gdb-multiarch
```

| Tool | Purpose |
|------|---------|
| `arm-none-eabi-gcc` | ARM bare-metal cross-compiler |
| `cmake` (3.20+) | Build system |
| `make` | Build backend |
| `openocd` | On-chip debugger / flash programmer |
| `gdb-multiarch` | Debugger with ARM support |

---

## Build

```bash
mkdir build && cd build
cmake .. -G "Unix Makefiles"
make -j$(nproc)          # produces build/firmware.elf
```

To build for a different board:

```bash
cmake .. -G "Unix Makefiles" -DBOARD=custom_pcb
```

---

## Flash

**Terminal 1 — start OpenOCD (keep it running):**

```bash
make load
```

**Terminal 2 — connect GDB, flash, and run:**

```bash
gdb-multiarch build/firmware.elf
(gdb) target remote localhost:3333
(gdb) load
(gdb) monitor reset halt
(gdb) continue
```

---

## Debug — FreeRTOS thread awareness

OpenOCD is configured with `-c "stm32f4x.cpu configure -rtos FreeRTOS"` in the `load`
target, so GDB can inspect individual tasks.

```
Ctrl+C                                  # halt the running firmware
(gdb) info threads                      # list all FreeRTOS tasks + state
(gdb) thread 2                          # switch to a task
(gdb) bt                                # show its call stack
(gdb) print g_cbuf                      # inspect the circular buffer
(gdb) x/8ub g_cbuf.data                 # dump buffer contents
(gdb) print uxQueueMessagesWaiting(g_cbuf.sem_free)   # semaphore count
```

---

## Project Structure

```
workspace/
├── BSP/                          # Board Support Package
│   ├── stm32f411_nucleo/
│   │   └── platform_config.h    # pin/bus assignments for this board
│   ├── Inc/led.h
│   ├── Src/led.c
│   └── CMakeLists.txt
├── Core/                         # Application tasks and logic
│   ├── Inc/
│   │   ├── FreeRTOSConfig.h
│   │   ├── cbuf.h
│   │   └── main.h
│   └── Src/
│       ├── main.c
│       └── cbuf.c
├── Drivers/                      # Portable device drivers (scaffolded)
├── HAL/                          # libopencm3 peripheral wrappers (scaffolded)
├── Startup/
│   └── stm32_startup.c           # Vector table and Reset_Handler
├── Linker/
│   └── stm32_ls.ld               # Flash/SRAM layout for STM32F411RE
├── cmake/
│   └── arm-none-eabi-gcc.cmake   # CMake cross-compiler toolchain file
├── ThirdParty/
│   ├── FreeRTOS/                 # FreeRTOS-Kernel (ARM_CM4F port, heap_4)
│   ├── libopencm3/               # Peripheral driver library
│   └── CMSIS/                    # ARM Cortex-M4 core headers (used by FreeRTOS port)
└── build/                        # Compiled output (generated — do not edit)
```

See `PROJECT_STRUCTURE.md` for a detailed explanation of every directory and
`ARCHITECTURE.md` for the layering rules.
