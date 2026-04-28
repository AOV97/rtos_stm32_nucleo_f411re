#include "cbuf.h"

void cbuf_init(CircBuf_t *cb)
{
    cb->head = 0;
    cb->tail = 0;

    /* sem_free: starts at CBUF_SIZE (all slots empty and available to write) */
    cb->sem_free = xSemaphoreCreateCounting(CBUF_SIZE, CBUF_SIZE);

    /* sem_used: starts at 0 (no items available to read yet) */
    cb->sem_used = xSemaphoreCreateCounting(CBUF_SIZE, 0);
}

void cbuf_write(CircBuf_t *cb, uint8_t val)
{
    /* Block here if the buffer is full — sem_free reaches 0 when all slots
     * are occupied.  The consumer signals it by calling Give after a read. */
    xSemaphoreTake(cb->sem_free, portMAX_DELAY);

    cb->data[cb->head] = val;
    cb->head = (cb->head + 1u) % CBUF_SIZE;

    /* Tell the consumer one more item is ready. */
    xSemaphoreGive(cb->sem_used);
}

uint8_t cbuf_read(CircBuf_t *cb)
{
    /* Block here if the buffer is empty — sem_used is 0 until the producer
     * writes something and calls Give. */
    xSemaphoreTake(cb->sem_used, portMAX_DELAY);

    uint8_t val = cb->data[cb->tail];
    cb->tail = (cb->tail + 1u) % CBUF_SIZE;

    /* Tell the producer one slot is free again. */
    xSemaphoreGive(cb->sem_free);

    return val;
}
