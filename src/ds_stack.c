/**
 * @file    ds_stack.c
 * @brief   固定大小栈实现
 * @version 1.0.0
 * @date    2026-05-30
 */

#include "inc/ds_stack.h"

/* ========================================================================
 * API函数实现
 * ======================================================================== */

ds_err_t ds_stack_init(ds_stack_t *stk, void **buffer, uint32_t capacity)
{
#if DS_ENABLE_CHECK
    if (stk == NULL || buffer == NULL) {
        return DS_ERR_NULL;
    }
    if (capacity == 0) {
        return DS_ERR_INVALID;
    }
#endif

    stk->buffer   = buffer;
    stk->capacity = capacity;
    stk->top      = 0;
    return DS_OK;
}

ds_err_t ds_stack_reset(ds_stack_t *stk)
{
#if DS_ENABLE_CHECK
    if (stk == NULL) {
        return DS_ERR_NULL;
    }
#endif

    stk->top = 0;
    return DS_OK;
}

ds_err_t ds_stack_push(ds_stack_t *stk, void *item)
{
#if DS_ENABLE_CHECK
    if (stk == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (ds_stack_is_full(stk)) {
        return DS_ERR_FULL;
    }

    stk->buffer[stk->top] = item;
    stk->top++;
    return DS_OK;
}

ds_err_t ds_stack_pop(ds_stack_t *stk, void **item)
{
#if DS_ENABLE_CHECK
    if (stk == NULL || item == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (ds_stack_is_empty(stk)) {
        return DS_ERR_EMPTY;
    }

    stk->top--;
    *item = stk->buffer[stk->top];
    return DS_OK;
}

ds_err_t ds_stack_peek(ds_stack_t *stk, void **item)
{
#if DS_ENABLE_CHECK
    if (stk == NULL || item == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (ds_stack_is_empty(stk)) {
        return DS_ERR_EMPTY;
    }

    *item = stk->buffer[stk->top - 1];
    return DS_OK;
}

uint32_t ds_stack_size(ds_stack_t *stk)
{
    if (stk == NULL) {
        return 0;
    }
    return stk->top;
}

int ds_stack_is_empty(ds_stack_t *stk)
{
    if (stk == NULL) {
        return 1;
    }
    return (stk->top == 0);
}

int ds_stack_is_full(ds_stack_t *stk)
{
    if (stk == NULL) {
        return 0;
    }
    return (stk->top == stk->capacity);
}
