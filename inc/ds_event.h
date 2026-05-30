/**
 * @file    ds_event.h
 * @brief   嵌入式轻量级数据结构库 - 简易事件总线
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 基于发布/订阅模式的轻量级事件总线：
 * - 支持多个事件通道，每个通道可有多个订阅者
 * - 事件传递为同步模式（发布时立即调用所有订阅者回调）
 * - 使用固定大小的订阅者池，零动态内存
 * - 适用于传感器数据分发、UI事件通知、模块间解耦通信等场景
 *
 * 使用示例：
 * @code
 *   ds_event_sub_t subs[32];
 *   ds_event_bus_t bus;
 *   ds_event_bus_init(&bus, subs, 32);
 *
 *   // 订阅温度事件
 *   ds_event_subscribe(&bus, EVENT_TEMP_UPDATE, my_temp_handler, NULL);
 *
 *   // 发布温度事件
 *   float temp = 25.5f;
 *   ds_event_publish(&bus, EVENT_TEMP_UPDATE, &temp);
 * @endcode
 */

#ifndef DS_EVENT_H
#define DS_EVENT_H

#include "ds_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 类型定义
 * ======================================================================== */

/** 事件ID类型 */
typedef uint32_t ds_event_id_t;

/** 无效事件ID */
#define DS_EVENT_ID_INVALID  0

/** 最大订阅者数量（单个事件通道） */
#define DS_EVENT_MAX_SUBS    8

/**
 * @brief 事件处理回调函数类型
 * @param event_id  事件ID
 * @param data      事件数据指针（由发布者提供）
 * @param user_data 订阅者自定义参数
 */
typedef void (*ds_event_handler_t)(ds_event_id_t event_id, void *data,
                                   void *user_data);

/**
 * @brief 单个订阅者信息
 */
typedef struct {
    ds_event_id_t event_id;        /**< 订阅的事件ID */
    ds_event_handler_t handler;    /**< 回调函数 */
    void *user_data;               /**< 用户数据 */
    uint8_t active;                /**< 是否激活（1=已订阅，0=空闲） */
} ds_event_sub_t;

/**
 * @brief 事件总线实例结构体
 */
typedef struct {
    ds_event_sub_t *subs;          /**< 订阅者数组（指向外部内存） */
    uint32_t capacity;             /**< 订阅者池总容量 */
    uint32_t active_count;         /**< 当前活跃订阅者数量 */
} ds_event_bus_t;

/* ========================================================================
 * API函数
 * ======================================================================== */

/**
 * @brief 初始化事件总线
 * @param bus       事件总线实例指针
 * @param subs      外部提供的订阅者数组
 * @param capacity  订阅者池总容量
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_INVALID 容量为0
 */
ds_err_t ds_event_bus_init(ds_event_bus_t *bus, ds_event_sub_t *subs,
                            uint32_t capacity);

/**
 * @brief 重置事件总线（取消所有订阅）
 * @param bus  事件总线实例指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_event_bus_reset(ds_event_bus_t *bus);

/**
 * @brief 订阅事件
 * @param bus        事件总线实例指针
 * @param event_id   要订阅的事件ID
 * @param handler    回调函数
 * @param user_data  用户数据
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_FULL 订阅池已满；
 *         DS_ERR_EXISTS 该handler已订阅同一事件
 */
ds_err_t ds_event_subscribe(ds_event_bus_t *bus, ds_event_id_t event_id,
                             ds_event_handler_t handler, void *user_data);

/**
 * @brief 取消订阅事件
 * @param bus      事件总线实例指针
 * @param event_id 事件ID
 * @param handler  要取消的回调函数
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_NOTFOUND 未找到该订阅
 */
ds_err_t ds_event_unsubscribe(ds_event_bus_t *bus, ds_event_id_t event_id,
                               ds_event_handler_t handler);

/**
 * @brief 发布事件（同步调用所有订阅者回调）
 * @param bus       事件总线实例指针
 * @param event_id  事件ID
 * @param data      事件数据（传递给所有订阅者）
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_event_publish(ds_event_bus_t *bus, ds_event_id_t event_id,
                           void *data);

/**
 * @brief 获取当前活跃订阅者数量
 * @param bus  事件总线实例指针
 * @return 活跃订阅者数；参数为空时返回0
 */
uint32_t ds_event_sub_count(ds_event_bus_t *bus);

#ifdef __cplusplus
}
#endif

#endif /* DS_EVENT_H */
