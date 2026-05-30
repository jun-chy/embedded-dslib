/**
 * @file    ds_event.c
 * @brief   简易事件总线实现
 * @version 1.0.0
 * @date    2026-05-30
 */

#include "inc/ds_event.h"

/* ========================================================================
 * API函数实现
 * ======================================================================== */

ds_err_t ds_event_bus_init(ds_event_bus_t *bus, ds_event_sub_t *subs,
                            uint32_t capacity)
{
#if DS_ENABLE_CHECK
    if (bus == NULL || subs == NULL) {
        return DS_ERR_NULL;
    }
    if (capacity == 0) {
        return DS_ERR_INVALID;
    }
#endif

    bus->subs         = subs;
    bus->capacity     = capacity;
    bus->active_count = 0;

    /* 初始化所有订阅者为非活跃状态 */
    for (uint32_t i = 0; i < capacity; i++) {
        bus->subs[i].active = 0;
        bus->subs[i].handler = NULL;
        bus->subs[i].event_id = DS_EVENT_ID_INVALID;
        bus->subs[i].user_data = NULL;
    }

    return DS_OK;
}

ds_err_t ds_event_bus_reset(ds_event_bus_t *bus)
{
#if DS_ENABLE_CHECK
    if (bus == NULL) {
        return DS_ERR_NULL;
    }
#endif

    for (uint32_t i = 0; i < bus->capacity; i++) {
        bus->subs[i].active = 0;
        bus->subs[i].handler = NULL;
        bus->subs[i].event_id = DS_EVENT_ID_INVALID;
        bus->subs[i].user_data = NULL;
    }
    bus->active_count = 0;
    return DS_OK;
}

ds_err_t ds_event_subscribe(ds_event_bus_t *bus, ds_event_id_t event_id,
                             ds_event_handler_t handler, void *user_data)
{
#if DS_ENABLE_CHECK
    if (bus == NULL || handler == NULL) {
        return DS_ERR_NULL;
    }
#endif

    /* 检查是否已订阅同一事件（防止重复） */
    for (uint32_t i = 0; i < bus->capacity; i++) {
        if (bus->subs[i].active &&
            bus->subs[i].event_id == event_id &&
            bus->subs[i].handler == handler) {
            return DS_ERR_EXISTS;
        }
    }

    /* 查找空闲槽位 */
    for (uint32_t i = 0; i < bus->capacity; i++) {
        if (!bus->subs[i].active) {
            bus->subs[i].event_id   = event_id;
            bus->subs[i].handler    = handler;
            bus->subs[i].user_data  = user_data;
            bus->subs[i].active     = 1;
            bus->active_count++;
            return DS_OK;
        }
    }

    return DS_ERR_FULL;
}

ds_err_t ds_event_unsubscribe(ds_event_bus_t *bus, ds_event_id_t event_id,
                               ds_event_handler_t handler)
{
#if DS_ENABLE_CHECK
    if (bus == NULL || handler == NULL) {
        return DS_ERR_NULL;
    }
#endif

    for (uint32_t i = 0; i < bus->capacity; i++) {
        if (bus->subs[i].active &&
            bus->subs[i].event_id == event_id &&
            bus->subs[i].handler == handler) {
            bus->subs[i].active     = 0;
            bus->subs[i].handler    = NULL;
            bus->subs[i].event_id   = DS_EVENT_ID_INVALID;
            bus->subs[i].user_data  = NULL;
            bus->active_count--;
            return DS_OK;
        }
    }

    return DS_ERR_NOTFOUND;
}

ds_err_t ds_event_publish(ds_event_bus_t *bus, ds_event_id_t event_id,
                           void *data)
{
#if DS_ENABLE_CHECK
    if (bus == NULL) {
        return DS_ERR_NULL;
    }
#endif

    /* 遍历所有订阅者，调用匹配的回调 */
    for (uint32_t i = 0; i < bus->capacity; i++) {
        if (bus->subs[i].active && bus->subs[i].event_id == event_id) {
            if (bus->subs[i].handler != NULL) {
                bus->subs[i].handler(event_id, data, bus->subs[i].user_data);
            }
        }
    }

    return DS_OK;
}

uint32_t ds_event_sub_count(ds_event_bus_t *bus)
{
    if (bus == NULL) {
        return 0;
    }
    return bus->active_count;
}
