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

void logger_init(void);
void logger_log(LogLevel_t level, const char *source, uint32_t line, const char *fmt, ...);
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
