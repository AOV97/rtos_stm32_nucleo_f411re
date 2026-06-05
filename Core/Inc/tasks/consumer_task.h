#ifndef CONSUMER_TASK_H_
#define CONSUMER_TASK_H_

/**
 * @brief FreeRTOS task that reads bytes from the shared circular buffer and drives LD2.
 *
 * Calls cbuf_read() every 500 ms. Blocks automatically when the buffer is empty
 * and resumes once the producer has written an item. Toggles the onboard green LED
 * (LD2, PA5) on each successful read to give a visible indication of activity.
 * Intended to run alongside producer_task to demonstrate the counting-semaphore
 * producer/consumer pattern.
 *
 * @param params  Pointer to the same CircBuf_t passed to producer_task, already
 *                initialized with cbuf_init().
 */
void consumer_task(void *params);

#endif /* CONSUMER_TASK_H_ */
