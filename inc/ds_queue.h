/**
 * @file    ds_queue.h
 * @brief   嵌入式轻量级数据结构库 - FIFO队列
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 基于环形缓冲区实现的先进先出（FIFO）队列：
 * - 底层复用环形缓冲区，天然支持批量操作
 * - 接口抽象为通用指针，可存储任意类型数据
 * - 适用于任务调度、消息传递、中断数据缓冲等场景
 *
 * @note 本实现存储的是数据指针（void*），而非数据副本。
 *       用户需确保队列引用的数据在出队前保持有效。
 *
 * 使用示例：
 * @code
 *   void *slot[16];  // 指针槽位数组
 *   ds_queue_t q;
 *   ds_queue_init(&q, slot, 16);
 *   int value = 42;
 *   ds_queue_enqueue(&q, &value);
 *   int *p = ds_queue_dequeue(&q);
 *   // *p == 42
 * @endcode
 */

#ifndef DS_QUEUE_H
#define DS_QUEUE_H

#include "ds_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 类型定义
 * ======================================================================== */

/**
 * @brief FIFO队列实例结构体
 *
 * 内部使用环形缓冲区逻辑存储void指针。
 * 容量必须为2的幂以获得最佳性能。
 */
typedef struct {
    void **buffer;       /**< 指针缓冲区（指向外部数组） */
    uint32_t capacity;   /**< 队列容量（必须为2的幂） */
    uint32_t mask;       /**< 容量掩码 */
    uint32_t head;       /**< 队头索引（出队位置） */
    uint32_t tail;       /**< 队尾索引（入队位置） */
    uint32_t count;      /**< 当前元素个数 */
} ds_queue_t;

/* ========================================================================
 * API函数
 * ======================================================================== */

/**
 * @brief 初始化队列
 * @param q         队列实例指针
 * @param buffer    外部提供的指针数组（大小必须为2的幂）
 * @param capacity  队列容量（必须为2的幂，且 >= 2）
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_INVALID 容量非法
 */
ds_err_t ds_queue_init(ds_queue_t *q, void **buffer, uint32_t capacity);

/**
 * @brief 重置队列（清空所有元素）
 * @param q  队列实例指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_queue_reset(ds_queue_t *q);

/**
 * @brief 入队操作（将元素加入队尾）
 * @param q    队列实例指针
 * @param item 数据指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_FULL 队列已满
 */
ds_err_t ds_queue_enqueue(ds_queue_t *q, void *item);

/**
 * @brief 出队操作（从队头取出元素）
 * @param q     队列实例指针
 * @param item  取出的数据指针存放地址
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_EMPTY 队列为空
 */
ds_err_t ds_queue_dequeue(ds_queue_t *q, void **item);

/**
 * @brief 查看队头元素（不出队）
 * @param q     队列实例指针
 * @param item  数据指针存放地址
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_EMPTY 队列为空
 */
ds_err_t ds_queue_peek(ds_queue_t *q, void **item);

/**
 * @brief 获取队列当前元素个数
 * @param q  队列实例指针
 * @return 元素个数；参数为空时返回0
 */
uint32_t ds_queue_size(ds_queue_t *q);

/**
 * @brief 判断队列是否为空
 * @param q  队列实例指针
 * @return 1=空，0=非空
 */
int ds_queue_is_empty(ds_queue_t *q);

/**
 * @brief 判断队列是否已满
 * @param q  队列实例指针
 * @return 1=满，0=未满
 */
int ds_queue_is_full(ds_queue_t *q);

#ifdef __cplusplus
}
#endif

#endif /* DS_QUEUE_H */
