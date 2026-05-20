# Logger Module

A thread-safe logging module for the FreeRTOS project. Any task can submit log entries; a dedicated logger task drains the queue and writes to the SD card via FatFS.

## Files

| File | Purpose |
|---|---|
| `Core/Inc/logger.h` | Public API — types, macros, function declarations |
| `Core/Src/logger.c` | Queue management, `logger_log()`, `logger_task()` |
| `Core/Src/syscalls.c` | Newlib syscall stubs required by `vsnprintf` |

## SD Card Layout

```
/
└── logs/
    └── system.log
```

The `/logs` directory is created automatically on first boot if it does not exist. Log entries are appended, so entries accumulate across reboots.

## Log Entry Format

```
[   1234ms][INFO ][GPS:42] Fix acquired
```

Fields in order: timestamp (ms since boot), severity level, source module and line number, message.

The timestamp is sourced from the FreeRTOS tick counter (`xTaskGetTickCount()`). It will read `0ms` for any entries queued before `vTaskStartScheduler()` is called. This placeholder will be replaced with real GPS time once the GPS module is integrated.

## Usage

Call `logger_init()` before starting the scheduler, then create the logger task:

```c
#include "logger.h"

logger_init();
xTaskCreate(logger_task, "Logger", configMINIMAL_STACK_SIZE * 4, NULL, 1, NULL);
```

From any task, use the four logging macros:

```c
LOG_DEBUG("GPS",    "Waiting for fix");
LOG_INFO("GPS",     "Fix acquired, satellites: %d", sat_count);
LOG_WARNING("POWER","Battery below 20%%");
LOG_ERROR("SD",     "Write failed, res=%d", res);
```

The first argument is a short module name that appears in the log output. The rest is a `printf`-style format string and optional arguments.

## Architecture

```
Any task                 Logger task
----------               -----------
LOG_INFO(...)            xQueueReceive (sleeps until entry arrives)
  → fills LogEntry_t          ↓
  → xQueueSend (non-blocking) format line with snprintf
                               ↓
                          f_puts → f_sync → SD card
```

**Why a queue?** FatFS is not thread-safe. By funnelling all writes through a single task, only one piece of code ever touches the filesystem. The queue (depth: 16 entries) lets other tasks submit logs without blocking, even if the SD write takes a few milliseconds.

**Why non-blocking send?** `xQueueSend` with timeout `0` means a task doing real work is never held up waiting for the logger. If the queue is full the entry is dropped, but the count is recorded and a recovery message is written to the log as soon as space becomes available (see Drop Counter below).

**Why `static` for `FATFS` and `FIL`?** Each is ~560 bytes. Placing them on the task stack would require a much larger stack allocation. `static` puts them in `.bss` (global RAM) instead.

## Compile-Time Log Level Filtering

Log levels below the configured threshold are compiled out entirely — no function call, no string in flash, zero runtime cost.

### Level constants

| Constant | Value | Meaning |
|---|---|---|
| `LOG_LVL_DEBUG` | 0 | Verbose debug output |
| `LOG_LVL_INFO` | 1 | Normal operational messages |
| `LOG_LVL_WARNING` | 2 | Unexpected but recoverable conditions |
| `LOG_LVL_ERROR` | 3 | Failures that need attention |
| `LOG_LVL_NONE` | 4 | Disables all logging |

### Setting the level at build time

```bash
# Development — log everything (default)
cmake -B build

# Production — errors only
cmake -B build -DLOG_LEVEL=LOG_LVL_ERROR

# Completely silent
cmake -B build -DLOG_LEVEL=LOG_LVL_NONE
```

The level is remembered in the CMake cache — you only need to pass `-DLOG_LEVEL` once per build directory.

### Default

`LOG_LVL_DEBUG` (all levels compiled in). Defined in two places that agree:
- `Core/CMakeLists.txt` — `set(LOG_LEVEL "LOG_LVL_DEBUG" CACHE STRING ...)`
- `Core/Inc/logger.h` — `#ifndef LOG_LEVEL` guard as a fallback

## Drop Counter

When the queue is full, the entry is discarded but a `volatile uint32_t s_dropped` counter is incremented in `logger.c`. The logger task checks this counter before writing each entry. If it is non-zero, a warning line is written first:

```
[   5432ms][WARN ][Logger:0] 3 entries dropped (queue overflow)
```

This tells you overflow happened, how many entries were lost, and roughly when recovery occurred. The content of the dropped entries is gone, but the event is no longer silent.

**A note on thread safety:** `s_dropped++` is a read-modify-write operation and is not strictly atomic — two tasks incrementing simultaneously could theoretically lose one count. For a drop counter this level of imprecision is acceptable; the goal is detecting overflow, not counting it with perfect accuracy. The reset (`s_dropped -= dropped`) subtracts only what was read rather than zeroing outright, which reduces (but does not eliminate) the race window.

## Future: GPS Timestamp Integration

The `timestamp_ms` field in `LogEntry_t` is currently filled by `xTaskGetTickCount()`. When the GPS module (GY-NEO6MV2 / NEO-6M) is integrated, replace the timestamp source in `logger_log()` with a GPS-derived Unix time or absolute clock value. No other part of the module needs to change.
