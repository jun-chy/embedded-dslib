/**
 * @file    ds_timer.c
 * @brief   软件定时器链表管理实现
 * @version 1.0.0
 * @date    2026-05-30
 */

#include "inc/ds_timer.h"

#include <string.h>

/* ========================================================================
 * 内部辅助函数
 * ======================================================================== */

/**
 * @brief 查找空闲定时器
 */
static ds_timer_t *find_free_timer(ds_timer_manager_t *mgr)
{
    for (uint32_t i = 0; i < mgr->capacity; i++) {
        if (mgr->timers[i].id == DS_TIMER_ID_INVALID) {
            return &mgr->timers[i];
        }
    }
    return NULL;
}

/* ========================================================================
 * API函数实现
 * ======================================================================== */

ds_err_t ds_timer_manager_init(ds_timer_manager_t *mgr, ds_timer_t *timers,
                                uint32_t capacity)
{
#if DS_ENABLE_CHECK
    if (mgr == NULL || timers == NULL) {
        return DS_ERR_NULL;
    }
    if (capacity == 0) {
        return DS_ERR_INVALID;
    }
#endif

    mgr->timers    = timers;
    mgr->capacity  = capacity;
    mgr->next_id   = 1;
    mgr->tick_count = 0;
    mgr->free_list = NULL;

    /* 初始化所有定时器为空闲状态 */
    memset(timers, 0, capacity * sizeof(ds_timer_t));
    for (uint32_t i = 0; i < capacity; i++) {
        timers[i].id    = DS_TIMER_ID_INVALID;
        timers[i].state  = DS_TIMER_STOPPED;
    }

    return DS_OK;
}

ds_timer_t *ds_timer_create(ds_timer_manager_t *mgr, const char *name,
                              uint32_t period, uint8_t is_periodic,
                              ds_timer_callback_t callback, void *user_data)
{
#if DS_ENABLE_CHECK
    if (mgr == NULL || callback == NULL) {
        return NULL;
    }
#endif

    ds_timer_t *timer = find_free_timer(mgr);
    if (timer == NULL) {
        return NULL;
    }

    /* 分配唯一ID */
    timer->id = mgr->next_id++;

    /* 设置名称 */
    if (name != NULL) {
        strncpy(timer->name, name, sizeof(timer->name) - 1);
        timer->name[sizeof(timer->name) - 1] = '\0';
    } else {
        timer->name[0] = '\0';
    }

    timer->period         = (period > 0) ? period : 1;
    timer->remaining      = timer->period;
    timer->repeat_count   = 0;  /* 0 = 无限循环 */
    timer->executed_count = 0;
    timer->is_periodic    = is_periodic;
    timer->state          = DS_TIMER_STOPPED;
    timer->callback       = callback;
    timer->user_data      = user_data;

    return timer;
}

ds_err_t ds_timer_start(ds_timer_t *timer)
{
#if DS_ENABLE_CHECK
    if (timer == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (timer->state == DS_TIMER_RUNNING) {
        return DS_OK;  /* 已在运行中，直接返回成功 */
    }

    timer->state     = DS_TIMER_RUNNING;
    timer->remaining = timer->period;
    return DS_OK;
}

ds_err_t ds_timer_stop(ds_timer_t *timer)
{
#if DS_ENABLE_CHECK
    if (timer == NULL) {
        return DS_ERR_NULL;
    }
#endif

    timer->state = DS_TIMER_STOPPED;
    return DS_OK;
}

ds_err_t ds_timer_delete(ds_timer_manager_t *mgr, ds_timer_t *timer)
{
#if DS_ENABLE_CHECK
    if (mgr == NULL || timer == NULL) {
        return DS_ERR_NULL;
    }
#endif

    /* 停止定时器 */
    timer->state = DS_TIMER_STOPPED;
    timer->id    = DS_TIMER_ID_INVALID;
    timer->callback = NULL;
    timer->user_data = NULL;
    timer->name[0]   = '\0';
    return DS_OK;
}

ds_err_t ds_timer_reset(ds_timer_t *timer)
{
#if DS_ENABLE_CHECK
    if (timer == NULL) {
        return DS_ERR_NULL;
    }
#endif

    ds_timer_stop(timer);
    timer->remaining      = timer->period;
    timer->executed_count = 0;
    return DS_OK;
}

ds_err_t ds_timer_tick(ds_timer_manager_t *mgr, uint32_t elapsed)
{
#if DS_ENABLE_CHECK
    if (mgr == NULL) {
        return DS_ERR_NULL;
    }
    if (elapsed == 0) {
        return DS_OK;
    }
#endif

    mgr->tick_count += elapsed;

    /* 遍历所有定时器，递减运行中的定时器剩余时间 */
    for (uint32_t i = 0; i < mgr->capacity; i++) {
        ds_timer_t *timer = &mgr->timers[i];

        if (timer->id == DS_TIMER_ID_INVALID) {
            continue;  /* 空闲槽位 */
        }
        if (timer->state != DS_TIMER_RUNNING) {
            continue;  /* 非运行状态 */
        }

        /* 递减剩余时间 */
        if (elapsed >= timer->remaining) {
            /* 定时器到期 */
            uint32_t overflow = elapsed - timer->remaining;
            timer->remaining = timer->period;

            /* 处理溢出：如果elapsed远大于period，需跳过多次 */
            if (overflow > 0 && overflow >= timer->period) {
                timer->remaining = timer->period - (overflow % timer->period);
                if (timer->remaining == 0) {
                    timer->remaining = timer->period;
                }
            } else if (overflow > 0) {
                timer->remaining = timer->period - overflow;
            }

            /* 调用回调 */
            timer->executed_count++;
            if (timer->callback != NULL) {
                timer->callback(timer->id, timer->user_data);
            }

            /* 检查是否需要停止（单次定时器或达到重复次数） */
            if (!timer->is_periodic) {
                timer->state = DS_TIMER_EXPIRED;
            } else if (timer->repeat_count > 0 &&
                       timer->executed_count >= timer->repeat_count) {
                timer->state = DS_TIMER_EXPIRED;
            }
        } else {
            timer->remaining -= elapsed;
        }
    }

    return DS_OK;
}

ds_timer_state_t ds_timer_state(ds_timer_t *timer)
{
    if (timer == NULL) {
        return DS_TIMER_STOPPED;
    }
    return timer->state;
}

uint32_t ds_timer_remaining(ds_timer_t *timer)
{
    if (timer == NULL) {
        return 0;
    }
    return timer->remaining;
}
