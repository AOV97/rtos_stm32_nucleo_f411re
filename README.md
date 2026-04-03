# STM32F411RE FreeRTOS LED Blink Demo

Bare-metal FreeRTOS application for the STM32F411RE Nucleo-64 board.
Four independent tasks blink the onboard LEDs at different rates, demonstrating
preemptive multitasking, task creation, and delay-based scheduling with FreeRTOS.

| Task   | LED    | GPIO | Rate  |
|--------|--------|------|-------|
| Task 1 | Green  | PD12 | 1 Hz  |
| Task 2 | Orange | PD13 | 1 Hz  |
| Task 3 | Blue   | PD15 | 4 Hz  |
| Task 4 | Red    | PD14 | 8 Hz  |

Built from scratch without an IDE using `arm-none-eabi-gcc`, a hand-written linker
script, and a custom startup file. FreeRTOS handles context switching via the
Cortex-M4 SysTick and PendSV exceptions.

---

## Requirements

- `arm-none-eabi-gcc` — ARM cross-compiler
- `cmake` — build system (3.20+)
- `make` — build tool
- `openocd` — on-chip debugger
- `stlink-tools` — ST-Link utilities
- `gdb-multiarch` — debugger

```bash
sudo apt install gcc-arm-none-eabi cmake make openocd stlink-tools gdb-multiarch
```

---

## Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)          # produces build/firmware.elf
```

---

## Flash & Debug

**Terminal 1 — start OpenOCD (keep it running):**
```bash
make load
```

**Terminal 2 — connect GDB, flash, and run:**
```bash
gdb-multiarch build/firmware.elf
(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) monitor reset halt
(gdb) continue
```

Press `Ctrl+C` in GDB to pause execution at any time.

---

## Useful GDB Commands

```
print pxCurrentTCB->pcTaskName   # name of the currently running task
info locals                       # local variables in current frame
bt                                # call stack (backtrace)
break task1_handler               # set a breakpoint
next                              # step over
step                              # step into
continue                          # resume
```

---

## Project Structure

```
workspace/
├── CMakeLists.txt          # Root build definition
├── cmake/
│   └── arm-none-eabi-gcc.cmake  # Cross-compiler toolchain file
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   └── FreeRTOSConfig.h     # FreeRTOS kernel configuration
│   └── Src/
│       └── main.c               # FreeRTOS tasks and application entry point
├── BSP/
│   ├── Inc/
│   │   └── led.h
│   └── Src/
│       └── led.c                # GPIO driver for onboard LEDs
├── Startup/
│   └── stm32_startup.c          # Vector table and Reset_Handler (.data/.bss init)
├── Linker/
│   └── stm32_ls.ld              # Linker script (FLASH/SRAM layout, heap section)
├── build/                       # Compiled output (generated)
└── ThirdParty/
    └── FreeRTOS/                # FreeRTOS-Kernel (ARM_CM4F port, heap_4)
```
