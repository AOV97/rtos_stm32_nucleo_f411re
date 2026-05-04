# Project Structure

This document explains every directory in the workspace, what it contains, and how it fits into the compilation pipeline that produces `firmware.elf`.

---

## How a build works (overview)

Running `cmake .. -G "Unix Makefiles" && make` does the following:

1. The CMake toolchain file (`cmake/arm-none-eabi-gcc.cmake`) tells CMake to use `arm-none-eabi-gcc` instead of the host compiler.
2. Each subdirectory with a `CMakeLists.txt` produces a **static library** (a `.a` file).
3. Those libraries are linked together with the startup object file to produce `build/firmware.elf`.
4. The linker script (`Linker/stm32_ls.ld`) controls exactly where each section of the ELF lands in the STM32's memory.

The dependency graph looks like this:

```
firmware.elf
├── Startup/stm32_startup.c   (compiled directly into the executable)
├── core         (libcore.a)
│   ├── main.c
│   ├── cbuf.c
│   └── → links bsp, freertos_kernel
├── bsp          (libbsp.a)
│   ├── led.c
│   └── → links opencm3
└── freertos_kernel  (libfreertos_kernel.a)
    └── → links freertos_kernel_port, opencm3 headers via freertos_config
```

`HAL` and `Drivers` are scaffolded but not yet linked — they will be added to this graph as peripherals and device drivers are implemented.

---

## Directory reference

### `cmake/`

**Contains:** `arm-none-eabi-gcc.cmake`

The CMake toolchain file. It runs before anything else and overrides CMake's defaults:

- Sets `CMAKE_C_COMPILER` to `arm-none-eabi-gcc` (cross-compiler for ARM bare-metal).
- Sets `CMAKE_SYSTEM_NAME` to `Generic`, which tells CMake this is a bare-metal target with no OS.
- Sets `CMAKE_TRY_COMPILE_TARGET_TYPE` to `STATIC_LIBRARY` so CMake doesn't try to link and run a test executable during configuration (which would fail on bare-metal).

Without this file, CMake would try to compile for your host machine (x86-64 Linux) instead of the STM32.

---

### `Linker/`

**Contains:** `stm32_ls.ld`

The linker script. It describes the STM32F411RE's physical memory layout and tells the linker where to place each section of compiled code:

| Section | Placed in | Purpose |
|---|---|---|
| `.isr_vector` | FLASH @ `0x08000000` | Vector table — must be first in flash |
| `.text` | FLASH | Compiled code and read-only data |
| `.data` | SRAM (loaded from FLASH) | Initialised global variables |
| `.bss` | SRAM | Zero-initialised globals |
| `.heap` | SRAM (NOLOAD) | FreeRTOS heap region (24 KB) |

It also exports symbols like `_sdata`, `_edata`, `_sbss`, `_ebss` that the startup code reads to know where to copy `.data` from flash and where to zero `.bss`.

The `AT> FLASH` on the `.data` section is what causes the linker to store the initial values of global variables in flash, even though they live in SRAM at runtime.

---

### `Startup/`

**Contains:** `stm32_startup.c`

The very first code that runs when the chip comes out of reset. It does three things:

1. **Defines the vector table** — a fixed array placed in `.isr_vector` at the start of flash. The first entry is the initial stack pointer value; the second is the address of `Reset_Handler`. All other entries are IRQ handler addresses, most of which are weak aliases to `Default_Handler` (an infinite loop) so they can be overridden elsewhere.

2. **Reset_Handler** — copies `.data` from flash to SRAM, then zeros `.bss`, then calls `main()`. This is the initialisation work that a normal OS would do for you.

3. **Weak fault/IRQ stubs** — handlers like `HardFault_Handler` and `SysTick_Handler` are declared as `weak` aliases. FreeRTOS overrides `SysTick_Handler`, `PendSV_Handler`, and `SVC_Handler` with its own implementations (mapped via `FreeRTOSConfig.h`). `main.c` overrides the fault handlers with its own infinite loops.

`stm32_startup.c` is compiled directly into `firmware.elf` in the top-level `CMakeLists.txt` (`add_executable(firmware.elf Startup/stm32_startup.c)`), not as a separate library, so it is always linked in first.

---

### `Core/`

**Contains:** `main.c`, `cbuf.c`, `cbuf.h`, `FreeRTOSConfig.h`, `main.h`

**CMake target:** `core` → `build/Core/libcore.a`

This is the application layer — your code.

**`main.c`** is the entry point after the startup code runs. It:
- Enables the FPU coprocessors (CP10/CP11) — mandatory before `vTaskStartScheduler()` because the ARM_CM4F FreeRTOS port saves and restores FPU registers on every context switch.
- Enables configurable fault handlers (MemManage, BusFault, UsageFault) so you get a specific fault instead of a generic HardFault when something goes wrong.
- Initialises the LED and the circular buffer.
- Creates the producer and consumer FreeRTOS tasks, then hands control to the scheduler.

**`cbuf.c` / `cbuf.h`** implement an 8-slot circular buffer shared between the producer and consumer tasks. It uses two counting semaphores — one tracking free slots, one tracking filled slots — so the producer blocks when the buffer is full and the consumer blocks when it is empty.

**`FreeRTOSConfig.h`** is the FreeRTOS configuration header. Every FreeRTOS project must supply one. It sets the CPU clock frequency (16 MHz HSI), tick rate (1000 Hz), heap size (24 KB), interrupt priority levels, and which optional FreeRTOS features are compiled in. FreeRTOS reads this file via the `freertos_config` CMake interface library defined in the top-level `CMakeLists.txt`.

**`main.h`** holds miscellaneous constants shared across the application.

---

### `BSP/`

**Contains:** `led.c`, `led.h`, `stm32f411_nucleo/platform_config.h`

**CMake target:** `bsp` → `build/BSP/libbsp.a`

BSP stands for Board Support Package. It is the hardware abstraction layer specific to the Nucleo-64 board — code that knows which GPIO pin the LED is on, so that `Core/` does not have to.

**`stm32f411_nucleo/platform_config.h`** is the single file that maps logical names to physical pins and peripherals for this board. `led.c` includes it and uses `LED_PORT` / `LED_PIN` instead of hardcoded register names, so moving to a different board only requires a new `platform_config.h`.

```c
#define LED_PORT    GPIOA
#define LED_PIN     GPIO5
```

**`led.c`** uses libopencm3 to:
- Enable the GPIOA peripheral clock (`rcc_periph_clock_enable`).
- Configure `LED_PORT` / `LED_PIN` as a push-pull output.
- Drive it high or low (`gpio_set` / `gpio_clear`).

`bsp` links `opencm3` as `PUBLIC`, which means the libopencm3 include directories and the `STM32F4` compile definition automatically propagate to `core` (which links `bsp`). That is why `main.c` can include `<libopencm3/cm3/scb.h>` without `Core/CMakeLists.txt` explicitly mentioning libopencm3.

The CMake `BOARD` variable (default: `stm32f411_nucleo`) selects which board subfolder is added to the include path. To target a different board:

```sh
cmake -DBOARD=custom_pcb ..
```

---

### `HAL/`

**Status:** scaffolded — no source files yet.

**Planned CMake target:** `hal` → `build/HAL/libhal.a`

This layer will contain thin wrappers around libopencm3 peripheral APIs: `hal_spi`, `hal_i2c`, `hal_uart`, etc. The HAL exposes generic `spi_transfer()` / `i2c_write()` style calls so that `Drivers/` above it never references libopencm3 types or register names directly.

The HAL is intentionally RTOS-unaware. It performs a single synchronous transaction and returns — any blocking or mutex logic belongs in the driver above it.

---

### `Drivers/`

**Status:** scaffolded — no source files yet.

**Planned layout:** one subdirectory per device (`sd_card/`, `oled/`, `bmp388/`, `neo6m/`), each with its own `CMakeLists.txt`.

Drivers implement device-specific protocol (reading BMP388 registers, writing OLED framebuffer, etc.) using HAL calls. They know the device's data format but not which physical pin or bus it is connected to — that comes from `platform_config.h`.

---

### `ThirdParty/`

External libraries. Nothing in here is code you wrote.

#### `ThirdParty/FreeRTOS/`

**CMake target:** `freertos_kernel` → `build/ThirdParty/FreeRTOS/libfreertos_kernel.a`

The FreeRTOS-Kernel source, cloned from GitHub. CMake builds it using FreeRTOS's own `CMakeLists.txt`. The key configuration choices made in the top-level `CMakeLists.txt`:

- `FREERTOS_PORT = GCC_ARM_CM4F` — selects the Cortex-M4 with hardware FPU port from `portable/GCC/ARM_CM4F/port.c`. This port contains the low-level assembly for context switching, SysTick setup, and interrupt priority management.
- `FREERTOS_HEAP = 4` — selects `heap_4.c`, a first-fit allocator with block merging. It uses a 24 KB array (`ucHeap[]`) that lives in the `.heap` section defined in the linker script.
- The `freertos_config` interface library tells FreeRTOS where to find `FreeRTOSConfig.h` (in `Core/Inc/`) and passes the CPU flags (`-mfpu=fpv4-sp-d16 -mfloat-abi=hard`) so the port compiles with hardware float support.

#### `ThirdParty/CMSIS/`

**No CMake target** — headers only.

The ARM CMSIS Core headers (`core_cm4.h`, `cmsis_gcc.h`, etc.). These are not used by your application code directly; they are used internally by the FreeRTOS ARM_CM4F port for Cortex-M intrinsics like `__get_BASEPRI()`, `__set_BASEPRI()`, and `__DSB()`.

libopencm3 does not use CMSIS, but FreeRTOS does, so this directory must remain.

#### `ThirdParty/libopencm3/`

**CMake target:** `opencm3` → `ThirdParty/libopencm3/lib/libopencm3_stm32f4.a`

An open-source peripheral driver library for ARM Cortex-M chips. It provides the register definitions and helper functions used to configure GPIO, RCC, USART, timers, and so on — without any of the "Handle struct" overhead that comes with ST's HAL.

The `CMakeLists.txt` wrapper uses CMake's `ExternalProject_Add` to invoke libopencm3's own Makefile (`make TARGETS=stm32/f4`) during the build. The resulting `.a` is then imported as a CMake `IMPORTED STATIC` library.

Two properties are set on the `opencm3` target and propagate to everything that links against it:
- `INTERFACE_INCLUDE_DIRECTORIES` — adds `libopencm3/include/` to the include path.
- `INTERFACE_COMPILE_DEFINITIONS STM32F4` — libopencm3's headers use this preprocessor define to select the correct family-specific register maps. Without it, every `#include <libopencm3/stm32/gpio.h>` would fail with `"stm32 family not defined."`.

---

### `build/`

**Generated — do not edit.**

CMake's out-of-tree build directory. All compiled object files (`.o`), static libraries (`.a`), and the final `firmware.elf` land here. The map file (`build/firmware.map`) is also written here and is useful for inspecting exactly which symbols ended up where in memory.

To do a clean rebuild: `rm -rf build/* && cmake .. -G "Unix Makefiles" && make`.

---

### Root-level files

| File | Purpose |
|---|---|
| `CMakeLists.txt` | Top-level CMake entry point. Defines CPU flags, `BOARD` variable, FreeRTOS options, links all subdirectories, and defines the `firmware.elf` link step and `load` target (OpenOCD). |
| `Linker/stm32_ls.ld` | Canonical linker script used by the CMake build. |
| `ARCHITECTURE.md` | Layering rules: how Core, Drivers, HAL, and BSP relate to each other. |
| `PROJECT_STRUCTURE.md` | This file — directory-by-directory reference. |
| `README.md` | Quick-start: build, flash, and debug instructions. |
