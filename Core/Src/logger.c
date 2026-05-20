#include "logger.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ff.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define LOG_DIR   "/logs"
#define LOG_FILE  "/logs/system.log"

static QueueHandle_t s_log_queue;
static volatile uint32_t s_dropped;

void logger_init(void)
{
    s_log_queue = xQueueCreate(LOG_QUEUE_DEPTH, sizeof(LogEntry_t));
}

void logger_log(LogLevel_t level, const char *source, uint32_t line, const char *fmt, ...)
{
    LogEntry_t entry;
    entry.level        = level;
    entry.source       = source;
    entry.line         = line;
    entry.timestamp_ms = xTaskGetTickCount();

    va_list args;
    va_start(args, fmt);
    vsnprintf(entry.message, sizeof(entry.message), fmt, args);
    va_end(args);

    if (xQueueSend(s_log_queue, &entry, 0) != pdTRUE)
        s_dropped++;
}

static const char *level_str(LogLevel_t level)
{
    switch (level) {
        case LOG_DEBUG:   return "DEBUG";
        case LOG_INFO:    return "INFO ";
        case LOG_WARNING: return "WARN ";
        case LOG_ERROR:   return "ERROR";
        default:          return "?????";
    }
}

void logger_task(void *params)
{
    (void)params;

    static FATFS fs;
    static FIL   file;
    LogEntry_t   entry;
    FRESULT      res;
    char         line_buf[160];

    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        for (;;) vTaskDelay(pdMS_TO_TICKS(500));
    }

    /* Create /logs directory — FR_EXIST is fine, the dir is already there. */
    res = f_mkdir(LOG_DIR);
    if (res != FR_OK && res != FR_EXIST) {
        f_unmount("");
        for (;;) vTaskDelay(pdMS_TO_TICKS(500));
    }

    res = f_open(&file, LOG_FILE, FA_WRITE | FA_OPEN_APPEND);
    if (res != FR_OK) {
        f_unmount("");
        for (;;) vTaskDelay(pdMS_TO_TICKS(500));
    }

    for (;;) {
        if (xQueueReceive(s_log_queue, &entry, portMAX_DELAY) == pdTRUE) {
            uint32_t dropped = s_dropped;
            if (dropped > 0) {
                s_dropped -= dropped;
                snprintf(line_buf, sizeof(line_buf),
                         "[%7lums][WARN ][Logger:0] %lu entries dropped (queue overflow)\n",
                         (unsigned long)xTaskGetTickCount(),
                         (unsigned long)dropped);
                f_puts(line_buf, &file);
            }

            snprintf(line_buf, sizeof(line_buf),
                     "[%7lums][%s][%s:%lu] %s\n",
                     (unsigned long)entry.timestamp_ms,
                     level_str(entry.level),
                     entry.source,
                     (unsigned long)entry.line,
                     entry.message);

            f_puts(line_buf, &file);
            f_sync(&file);
        }
    }
}
