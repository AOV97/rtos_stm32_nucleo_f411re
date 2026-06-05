#ifndef PRODUCER_TASK_H_
#define PRODUCER_TASK_H_

/**
 * @brief FreeRTOS task that writes incrementing bytes into the shared circular buffer.
 *
 * Calls cbuf_write() every 100 ms. Blocks automatically when the buffer is full
 * (all CBUF_SIZE slots occupied) and resumes once the consumer has freed a slot.
 * Intended to run alongside consumer_task to demonstrate the counting-semaphore
 * producer/consumer pattern.
 *
 * @param params  Pointer to a CircBuf_t that has been initialized with cbuf_init().
 */
void producer_task(void *params);

#endif /* PRODUCER_TASK_H_ */
