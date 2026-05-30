/**
 * @file    ds_timer.h
 * @brief   嵌入式轻量级数据结构库 - 软件定时器链表管理
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 基于双向链表的软件定时器管理模块：
 * - 支持单次定时器和周期定时器
 * - 定时器按超时时间排序，提高轮询效率
 * - 用户需周期调用 ds_timer_tick() 驱动时间推进
 * - 适用于RTOS/裸机环境下的周期任务调度
 *
 * 时间基准：
 * - 本模块使用抽象的"tick"作为时间单位
 * - 用户可自行定义tick对应的时间（如1ms、10ms等）
 *
 * 使用示例：
 * @code
 *   ds_timer_manager_t mgr;
 *   ds_timer_t timers[8];
 *   ds_timer_manager_init(&mgr, timers, 8);
 *
 *   ds_timer_t *t = ds_timer_create(&mgr, "led_blink", 500, 1,
 *                                    my_callback, NULL);
 *   ds_timer_start(t);
 *
 *   // 在主循环中周期调用（假设每1ms调用一次）
 *   while (1) {
 *       ds_timer_tick(&mgr, 1);
 *       // ...
 *   }
 * @endcode
 */

#ifndef DS_TIMER_H
#define DS_TIMER_H

#include "ds_common.h"
#include "ds_linkedlist.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 类型定义
 * ======================================================================== */

/** 定时器ID类型（无效ID为0） */
typedef uint32_t ds_timer_id_t;

#define DS_TIMER_ID_INVALID  0  /**< 无效定时器ID */

/** 定时器状态 */
typedef enum {
    DS_TIMER_STOPPED = 0,   /**< 已停止 */
    DS_TIMER_RUNNING = 1,   /**< 运行中 */
    DS_TIMER_EXPIRED = 2,   /**< 已到期（单次定时器使用后） */
} ds_timer_state_t;

/**
 * @brief 定时器回调函数类型
 * @param timer_id  触发的定时器ID
 * @param user_data 用户自定义参数
 */
typedef void (*ds_timer_callback_t)(ds_timer_id_t timer_id, void *user_data);

/**
 * @brief 定时器结构体
 */
typedef struct ds_timer {
    ds_timer_id_t id;               /**< 定时器ID（唯一标识） */
    char name[16];                   /**< 定时器名称（用于调试） */
    uint32_t period;                 /**< 定时周期（tick数） */
    uint32_t remaining;             /**< 剩余tick数 */
    uint32_t repeat_count;           /**< 重复次数（0=无限循环） */
    uint32_t executed_count;        /**< 已执行次数 */
    uint8_t  is_periodic;            /**< 是否为周期定时器（1=周期，0=单次） */
    ds_timer_state_t state;          /**< 当前状态 */
    ds_timer_callback_t callback;    /**< 回调函数 */
    void *user_data;                 /**< 用户数据 */
    ds_list_node_t node;            /**< 链表节点（嵌入） */
    struct ds_timer *next;           /**< 下一个定时器（用于数组管理） */
} ds_timer_t;

/**
 * @brief 定时器管理器结构体
 */
typedef struct {
    ds_timer_t *timers;          /**< 定时器数组（指向外部内存） */
    uint32_t capacity;           /**< 定时器池容量 */
    uint32_t next_id;            /**< 下一个分配的ID */
    uint32_t tick_count;         /**< 全局tick计数器 */
    ds_timer_t *free_list;       /**< 空闲定时器链表 */
} ds_timer_manager_t;

/* ========================================================================
 * API函数 - 管理器操作
 * ======================================================================== */

/**
 * @brief 初始化定时器管理器
 * @param mgr       管理器实例指针
 * @param timers    外部提供的定时器数组
 * @param capacity  定时器池容量
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_INVALID 容量为0
 */
ds_err_t ds_timer_manager_init(ds_timer_manager_t *mgr, ds_timer_t *timers,
                                uint32_t capacity);

/**
 * @brief 创建定时器（从池中分配一个空闲定时器）
 * @param mgr        管理器实例指针
 * @param name       定时器名称
 * @param period     定时周期（tick数）
 * @param is_periodic 是否为周期定时器（1=周期，0=单次）
 * @param callback   回调函数
 * @param user_data  用户数据
 * @return 定时器指针；失败时返回NULL
 */
ds_timer_t *ds_timer_create(ds_timer_manager_t *mgr, const char *name,
                             uint32_t period, uint8_t is_periodic,
                             ds_timer_callback_t callback, void *user_data);

/**
 * @brief 启动定时器
 * @param timer  定时器指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_timer_start(ds_timer_t *timer);

/**
 * @brief 停止定时器
 * @param timer  定时器指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_timer_stop(ds_timer_t *timer);

/**
 * @brief 删除定时器（归还到池中）
 * @param mgr    管理器实例指针
 * @param timer  定时器指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_timer_delete(ds_timer_manager_t *mgr, ds_timer_t *timer);

/**
 * @brief 重置定时器（停止并清零计数器）
 * @param timer  定时器指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_timer_reset(ds_timer_t *timer);

/**
 * @brief 推进定时器时间（在主循环/中断中周期调用）
 * @param mgr      管理器实例指针
 * @param elapsed  经过的tick数
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 *
 * @note 此函数会遍历所有运行中的定时器，递减其剩余时间，
 *       对已到期的定时器调用回调函数。
 */
ds_err_t ds_timer_tick(ds_timer_manager_t *mgr, uint32_t elapsed);

/**
 * @brief 获取定时器状态
 * @param timer  定时器指针
 * @return 定时器状态；参数为空时返回 DS_TIMER_STOPPED
 */
ds_timer_state_t ds_timer_state(ds_timer_t *timer);

/**
 * @brief 获取定时器剩余时间
 * @param timer  定时器指针
 * @return 剩余tick数；参数为空时返回0
 */
uint32_t ds_timer_remaining(ds_timer_t *timer);

#ifdef __cplusplus
}
#endif

#endif /* DS_TIMER_H */
