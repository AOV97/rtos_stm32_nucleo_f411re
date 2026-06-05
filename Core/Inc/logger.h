#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>

#define LOG_MAX_MSG_LEN  96
#define LOG_QUEUE_DEPTH  16

/* Integer constants used by #if for compile-time filtering */
#define LOG_LVL_DEBUG    0
#define LOG_LVL_INFO     1
#define LOG_LVL_WARNING  2
#define LOG_LVL_ERROR    3
#define LOG_LVL_NONE     4

/* Default: log everything. Override at build time with -DLOG_LEVEL=LOG_LVL_INFO etc. */
#ifndef LOG_LEVEL
#define LOG_LEVEL  LOG_LVL_DEBUG
#endif

typedef enum {
    LOG_DEBUG   = 0,
    LOG_INFO    = 1,
    LOG_WARNING = 2,
    LOG_ERROR   = 3,
} LogLevel_t;

typedef struct {
    LogLevel_t  level;
    const char *source;
    uint32_t    line;
    uint32_t    timestamp_ms;
    char        message[LOG_MAX_MSG_LEN];
} LogEntry_t;

/**
 * @brief Creates the internal log message queue.
 *
 * Allocates a FreeRTOS queue of LOG_QUEUE_DEPTH entries. Must be called once
 * before any LOG_* macro or logger_task is used. Calling before the scheduler
 * starts is safe because queue creation does not require the scheduler to be
 * running.
 */
void logger_init(void);

/**
 * @brief Formats a log message and enqueues it for the logger task to print.
 *
 * Captures the current FreeRTOS tick as a timestamp, formats the message with
 * printf-style arguments into a LogEntry_t, and posts it to the queue without
 * blocking (entries are dropped if the queue is full). Not usually called
 * directly — use the LOG_DEBUG / LOG_INFO / LOG_WARNING / LOG_ERROR macros,
 * which also strip calls at compile time based on LOG_LEVEL.
 *
 * @param level   Severity of the message (LOG_DEBUG, LOG_INFO, etc.).
 * @param source  Null-terminated string naming the calling module (e.g. "Main").
 * @param line    Source line number — pass __LINE__ or use the macros.
 * @param fmt     printf-style format string followed by optional arguments.
 */
void logger_log(LogLevel_t level, const char *source, uint32_t line,
                const char *fmt, ...);

/**
 * @brief FreeRTOS task that dequeues and prints log entries over UART/semihosting.
 *
 * Blocks on the log queue indefinitely and prints each entry as it arrives.
 * Create with xTaskCreate at priority 1 (low) so logging never preempts
 * application tasks. Pass NULL as pvParameters.
 *
 * @param params  Unused. Pass NULL when creating with xTaskCreate.
 */
void logger_task(void *params);

#if LOG_LEVEL <= LOG_LVL_DEBUG
#define LOG_DEBUG(src, fmt, ...)    logger_log(LOG_DEBUG,   (src), __LINE__, (fmt), ##__VA_ARGS__)
#else
#define LOG_DEBUG(src, fmt, ...)    ((void)0)
#endif

#if LOG_LEVEL <= LOG_LVL_INFO
#define LOG_INFO(src, fmt, ...)     logger_log(LOG_INFO,    (src), __LINE__, (fmt), ##__VA_ARGS__)
#else
#define LOG_INFO(src, fmt, ...)     ((void)0)
#endif

#if LOG_LEVEL <= LOG_LVL_WARNING
#define LOG_WARNING(src, fmt, ...)  logger_log(LOG_WARNING, (src), __LINE__, (fmt), ##__VA_ARGS__)
#else
#define LOG_WARNING(src, fmt, ...)  ((void)0)
#endif

#if LOG_LEVEL <= LOG_LVL_ERROR
#define LOG_ERROR(src, fmt, ...)    logger_log(LOG_ERROR,   (src), __LINE__, (fmt), ##__VA_ARGS__)
#else
#define LOG_ERROR(src, fmt, ...)    ((void)0)
#endif

#endif
