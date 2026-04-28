#ifndef CBUF_H
#define CBUF_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

#define CBUF_SIZE 8u   /* keep small so the buffer fills quickly in the demo */

typedef struct {
    uint8_t           data[CBUF_SIZE];
    uint32_t          head;       /* write index — owned exclusively by the producer */
    uint32_t          tail;       /* read  index — owned exclusively by the consumer */
    SemaphoreHandle_t sem_free;   /* counts free slots:  init = CBUF_SIZE */
    SemaphoreHandle_t sem_used;   /* counts used slots:  init = 0         */
} CircBuf_t;

/* Must be called once before any task uses the buffer. */
void    cbuf_init(CircBuf_t *cb);

/* Block until a free slot is available, then write val. */
void    cbuf_write(CircBuf_t *cb, uint8_t val);

/* Block until an item is available, then return it. */
uint8_t cbuf_read(CircBuf_t *cb);

#endif /* CBUF_H */
