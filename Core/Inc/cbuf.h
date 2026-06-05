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

/**
 * @brief Initializes a circular buffer and creates its two counting semaphores.
 *
 * Sets head and tail to zero, creates sem_free (initial count = CBUF_SIZE) and
 * sem_used (initial count = 0). Must be called once from a task or before the
 * scheduler starts, before cbuf_write or cbuf_read are used.
 *
 * @param cb  Pointer to an uninitialized CircBuf_t to set up.
 */
void cbuf_init(CircBuf_t *cb);

/**
 * @brief Blocks until a free slot is available, then writes one byte.
 *
 * Takes sem_free (blocks if the buffer is full), writes val at the current
 * head index, advances head with wrap-around, then gives sem_used to wake
 * any waiting consumer. Safe to call from exactly one producer task.
 *
 * @param cb   Pointer to an initialized CircBuf_t.
 * @param val  Byte value to enqueue.
 */
void cbuf_write(CircBuf_t *cb, uint8_t val);

/**
 * @brief Blocks until an item is available, then returns it.
 *
 * Takes sem_used (blocks if the buffer is empty), reads the byte at the
 * current tail index, advances tail with wrap-around, then gives sem_free
 * to wake any waiting producer. Safe to call from exactly one consumer task.
 *
 * @param cb  Pointer to an initialized CircBuf_t.
 *
 * @return The next byte dequeued from the buffer.
 */
uint8_t cbuf_read(CircBuf_t *cb);

#endif /* CBUF_H */
