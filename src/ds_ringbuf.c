/**
 * @file    ds_ringbuf.c
 * @brief   环形缓冲区实现
 * @version 1.0.0
 * @date    2026-05-30
 */

#include "inc/ds_ringbuf.h"

/* ========================================================================
 * 内部辅助函数
 * ======================================================================== */

#if DS_ENABLE_CHECK
/**
 * @brief 校验环形缓冲区参数是否合法
 */
static ds_err_t rb_check(const ds_ringbuf_t *rb)
{
    if (rb == NULL) {
        return DS_ERR_NULL;
    }
    return DS_OK;
}
#endif

/* ========================================================================
 * API函数实现
 * ======================================================================== */

ds_err_t ds_ringbuf_init(ds_ringbuf_t *rb, uint8_t *buffer, uint32_t capacity)
{
#if DS_ENABLE_CHECK
    if (rb == NULL || buffer == NULL) {
        return DS_ERR_NULL;
    }
    if (capacity < 2 || !DS_IS_POWER_OF_TWO(capacity)) {
        return DS_ERR_INVALID;
    }
#endif

    rb->buffer   = buffer;
    rb->capacity = capacity;
    rb->mask     = capacity - 1;
    rb->head     = 0;
    rb->tail     = 0;
    return DS_OK;
}

ds_err_t ds_ringbuf_reset(ds_ringbuf_t *rb)
{
#if DS_ENABLE_CHECK
    if (rb == NULL) {
        return DS_ERR_NULL;
    }
#endif

    rb->head = 0;
    rb->tail = 0;
    return DS_OK;
}

ds_err_t ds_ringbuf_write(ds_ringbuf_t *rb, uint8_t data)
{
#if DS_ENABLE_CHECK
    if (rb == NULL) {
        return DS_ERR_NULL;
    }
#endif

    /* 判断是否已满 */
    if (ds_ringbuf_is_full(rb)) {
        return DS_ERR_FULL;
    }

    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) & rb->mask;
    return DS_OK;
}

ds_err_t ds_ringbuf_read(ds_ringbuf_t *rb, uint8_t *data)
{
#if DS_ENABLE_CHECK
    if (rb == NULL || data == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (ds_ringbuf_is_empty(rb)) {
        return DS_ERR_EMPTY;
    }

    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) & rb->mask;
    return DS_OK;
}

ds_err_t ds_ringbuf_write_batch(ds_ringbuf_t *rb, const uint8_t *data,
                                uint32_t len, uint32_t *written)
{
#if DS_ENABLE_CHECK
    if (rb == NULL || data == NULL) {
        return DS_ERR_NULL;
    }
#endif

    uint32_t free = ds_ringbuf_free_space(rb);
    uint32_t count = DS_MIN(len, free);

    if (written != NULL) {
        *written = count;
    }

    for (uint32_t i = 0; i < count; i++) {
        rb->buffer[rb->head] = data[i];
        rb->head = (rb->head + 1) & rb->mask;
    }

    return DS_OK;
}

ds_err_t ds_ringbuf_read_batch(ds_ringbuf_t *rb, uint8_t *buf,
                                uint32_t buf_size, uint32_t *read)
{
#if DS_ENABLE_CHECK
    if (rb == NULL || buf == NULL) {
        return DS_ERR_NULL;
    }
#endif

    uint32_t avail = ds_ringbuf_available(rb);
    uint32_t count = DS_MIN(buf_size, avail);

    if (read != NULL) {
        *read = count;
    }

    for (uint32_t i = 0; i < count; i++) {
        buf[i] = rb->buffer[rb->tail];
        rb->tail = (rb->tail + 1) & rb->mask;
    }

    return DS_OK;
}

ds_err_t ds_ringbuf_peek(ds_ringbuf_t *rb, uint8_t *data)
{
#if DS_ENABLE_CHECK
    if (rb == NULL || data == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (ds_ringbuf_is_empty(rb)) {
        return DS_ERR_EMPTY;
    }

    *data = rb->buffer[rb->tail];
    return DS_OK;
}

uint32_t ds_ringbuf_available(ds_ringbuf_t *rb)
{
    if (rb == NULL) {
        return 0;
    }
    return (rb->head - rb->tail) & rb->mask;
}

uint32_t ds_ringbuf_free_space(ds_ringbuf_t *rb)
{
    if (rb == NULL) {
        return 0;
    }
    return rb->capacity - ds_ringbuf_available(rb);
}

int ds_ringbuf_is_empty(ds_ringbuf_t *rb)
{
    if (rb == NULL) {
        return 1;
    }
    return (rb->head == rb->tail);
}

int ds_ringbuf_is_full(ds_ringbuf_t *rb)
{
    if (rb == NULL) {
        return 0;
    }
    return (((rb->head + 1) & rb->mask) == rb->tail);
}
