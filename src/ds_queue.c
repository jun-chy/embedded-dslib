/**
 * @file    ds_queue.c
 * @brief   FIFO队列实现
 * @version 1.0.0
 * @date    2026-05-30
 */

#include "inc/ds_queue.h"

/* ========================================================================
 * API函数实现
 * ======================================================================== */

ds_err_t ds_queue_init(ds_queue_t *q, void **buffer, uint32_t capacity)
{
#if DS_ENABLE_CHECK
    if (q == NULL || buffer == NULL) {
        return DS_ERR_NULL;
    }
    if (capacity < 2 || !DS_IS_POWER_OF_TWO(capacity)) {
        return DS_ERR_INVALID;
    }
#endif

    q->buffer   = buffer;
    q->capacity = capacity;
    q->mask     = capacity - 1;
    q->head     = 0;
    q->tail     = 0;
    q->count    = 0;
    return DS_OK;
}

ds_err_t ds_queue_reset(ds_queue_t *q)
{
#if DS_ENABLE_CHECK
    if (q == NULL) {
        return DS_ERR_NULL;
    }
#endif

    q->head  = 0;
    q->tail  = 0;
    q->count = 0;
    return DS_OK;
}

ds_err_t ds_queue_enqueue(ds_queue_t *q, void *item)
{
#if DS_ENABLE_CHECK
    if (q == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (ds_queue_is_full(q)) {
        return DS_ERR_FULL;
    }

    q->buffer[q->tail] = item;
    q->tail = (q->tail + 1) & q->mask;
    q->count++;
    return DS_OK;
}

ds_err_t ds_queue_dequeue(ds_queue_t *q, void **item)
{
#if DS_ENABLE_CHECK
    if (q == NULL || item == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (ds_queue_is_empty(q)) {
        return DS_ERR_EMPTY;
    }

    *item = q->buffer[q->head];
    q->head = (q->head + 1) & q->mask;
    q->count--;
    return DS_OK;
}

ds_err_t ds_queue_peek(ds_queue_t *q, void **item)
{
#if DS_ENABLE_CHECK
    if (q == NULL || item == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (ds_queue_is_empty(q)) {
        return DS_ERR_EMPTY;
    }

    *item = q->buffer[q->head];
    return DS_OK;
}

uint32_t ds_queue_size(ds_queue_t *q)
{
    if (q == NULL) {
        return 0;
    }
    return q->count;
}

int ds_queue_is_empty(ds_queue_t *q)
{
    if (q == NULL) {
        return 1;
    }
    return (q->count == 0);
}

int ds_queue_is_full(ds_queue_t *q)
{
    if (q == NULL) {
        return 0;
    }
    return (q->count == q->capacity);
}
