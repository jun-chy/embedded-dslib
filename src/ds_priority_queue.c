/**
 * @file    ds_priority_queue.c
 * @brief   优先级队列实现（二叉小顶堆）
 * @version 1.0.0
 * @date    2026-05-30
 */

#include "inc/ds_priority_queue.h"

/* ========================================================================
 * 内部辅助函数
 * ======================================================================== */

/**
 * @brief 交换堆中两个元素
 */
static void pq_swap(ds_pqueue_t *pq, uint32_t a, uint32_t b)
{
    void *tmp = pq->buffer[a];
    pq->buffer[a] = pq->buffer[b];
    pq->buffer[b] = tmp;
}

/**
 * @brief 上浮操作（从指定位置向上调整堆序）
 */
static void pq_sift_up(ds_pqueue_t *pq, uint32_t idx)
{
    while (idx > 0) {
        uint32_t parent = (idx - 1) / 2;
        if (pq->compare(pq->buffer[idx], pq->buffer[parent]) < 0) {
            pq_swap(pq, idx, parent);
            idx = parent;
        } else {
            break;
        }
    }
}

/**
 * @brief 下沉操作（从指定位置向下调整堆序）
 */
static void pq_sift_down(ds_pqueue_t *pq, uint32_t idx)
{
    uint32_t size = pq->size;

    while (1) {
        uint32_t smallest = idx;
        uint32_t left  = 2 * idx + 1;
        uint32_t right = 2 * idx + 2;

        if (left < size &&
            pq->compare(pq->buffer[left], pq->buffer[smallest]) < 0) {
            smallest = left;
        }
        if (right < size &&
            pq->compare(pq->buffer[right], pq->buffer[smallest]) < 0) {
            smallest = right;
        }

        if (smallest != idx) {
            pq_swap(pq, idx, smallest);
            idx = smallest;
        } else {
            break;
        }
    }
}

/* ========================================================================
 * API函数实现
 * ======================================================================== */

ds_err_t ds_pqueue_init(ds_pqueue_t *pq, void **buffer, uint32_t capacity,
                         ds_pqueue_compare_t compare)
{
#if DS_ENABLE_CHECK
    if (pq == NULL || buffer == NULL || compare == NULL) {
        return DS_ERR_NULL;
    }
    if (capacity == 0) {
        return DS_ERR_INVALID;
    }
#endif

    pq->buffer   = buffer;
    pq->capacity = capacity;
    pq->size     = 0;
    pq->compare  = compare;
    return DS_OK;
}

ds_err_t ds_pqueue_reset(ds_pqueue_t *pq)
{
#if DS_ENABLE_CHECK
    if (pq == NULL) {
        return DS_ERR_NULL;
    }
#endif

    pq->size = 0;
    return DS_OK;
}

ds_err_t ds_pqueue_push(ds_pqueue_t *pq, void *item)
{
#if DS_ENABLE_CHECK
    if (pq == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (ds_pqueue_is_full(pq)) {
        return DS_ERR_FULL;
    }

    /* 放在数组末尾，然后上浮 */
    pq->buffer[pq->size] = item;
    pq_sift_up(pq, pq->size);
    pq->size++;
    return DS_OK;
}

ds_err_t ds_pqueue_pop(ds_pqueue_t *pq, void **item)
{
#if DS_ENABLE_CHECK
    if (pq == NULL || item == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (ds_pqueue_is_empty(pq)) {
        return DS_ERR_EMPTY;
    }

    /* 取出堆顶 */
    *item = pq->buffer[0];
    pq->size--;

    if (pq->size > 0) {
        /* 将最后一个元素移到堆顶，然后下沉 */
        pq->buffer[0] = pq->buffer[pq->size];
        pq_sift_down(pq, 0);
    }

    return DS_OK;
}

ds_err_t ds_pqueue_peek(ds_pqueue_t *pq, void **item)
{
#if DS_ENABLE_CHECK
    if (pq == NULL || item == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (ds_pqueue_is_empty(pq)) {
        return DS_ERR_EMPTY;
    }

    *item = pq->buffer[0];
    return DS_OK;
}

uint32_t ds_pqueue_size(ds_pqueue_t *pq)
{
    if (pq == NULL) {
        return 0;
    }
    return pq->size;
}

int ds_pqueue_is_empty(ds_pqueue_t *pq)
{
    if (pq == NULL) {
        return 1;
    }
    return (pq->size == 0);
}

int ds_pqueue_is_full(ds_pqueue_t *pq)
{
    if (pq == NULL) {
        return 0;
    }
    return (pq->size == pq->capacity);
}
