/**
 * @file    ds_priority_queue.h
 * @brief   嵌入式轻量级数据结构库 - 优先级队列
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 基于二叉小顶堆的优先级队列：
 * - 自动维护堆序，取出时始终获得最小优先级元素
 * - 使用void指针数组，支持存储任意类型
 * - 用户需提供优先级比较函数
 * - 所有操作：插入O(log n)，取出O(log n)，查看O(1)
 * - 适用于任务调度、事件优先级管理、Dijkstra算法等场景
 *
 * 使用示例：
 * @code
 *   void *heap[32];
 *   ds_pqueue_t pq;
 *   ds_pqueue_init(&pq, heap, 32, my_compare_func);
 *   ds_pqueue_push(&pq, &task_a);  // 自动维护堆序
 *   void *top = ds_pqueue_pop(&pq);  // 取出最高优先级
 * @endcode
 */

#ifndef DS_PRIORITY_QUEUE_H
#define DS_PRIORITY_QUEUE_H

#include "ds_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 类型定义
 * ======================================================================== */

/**
 * @brief 优先级比较函数类型
 * @param a  第一个元素指针
 * @param b  第二个元素指针
 * @return <0 表示a优先级更高（a应排在前面）
 * @return  0 表示优先级相同
 * @return >0 表示b优先级更高（b应排在前面）
 */
typedef int (*ds_pqueue_compare_t)(const void *a, const void *b);

/**
 * @brief 优先级队列实例结构体
 *
 * 内部使用数组实现的二叉小顶堆。
 * parent(i) = (i-1)/2
 * left(i)   = 2*i+1
 * right(i)  = 2*i+2
 */
typedef struct {
    void **buffer;                  /**< 堆数组（指向外部内存） */
    uint32_t capacity;              /**< 堆最大容量 */
    uint32_t size;                  /**< 当前元素个数 */
    ds_pqueue_compare_t compare;    /**< 优先级比较函数 */
} ds_pqueue_t;

/* ========================================================================
 * API函数
 * ======================================================================== */

/**
 * @brief 初始化优先级队列
 * @param pq       优先级队列实例指针
 * @param buffer   外部提供的指针数组
 * @param capacity 堆最大容量
 * @param compare  优先级比较函数（不可为NULL）
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_INVALID 容量为0
 */
ds_err_t ds_pqueue_init(ds_pqueue_t *pq, void **buffer, uint32_t capacity,
                         ds_pqueue_compare_t compare);

/**
 * @brief 重置优先级队列
 * @param pq  优先级队列实例指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_pqueue_reset(ds_pqueue_t *pq);

/**
 * @brief 入堆操作（插入元素并维护堆序）
 * @param pq    优先级队列实例指针
 * @param item  数据指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_FULL 堆已满
 */
ds_err_t ds_pqueue_push(ds_pqueue_t *pq, void *item);

/**
 * @brief 出堆操作（取出并移除堆顶元素）
 * @param pq    优先级队列实例指针
 * @param item  取出的数据指针存放地址
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_EMPTY 堆为空
 */
ds_err_t ds_pqueue_pop(ds_pqueue_t *pq, void **item);

/**
 * @brief 查看堆顶元素（不移除）
 * @param pq    优先级队列实例指针
 * @param item  数据指针存放地址
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_EMPTY 堆为空
 */
ds_err_t ds_pqueue_peek(ds_pqueue_t *pq, void **item);

/**
 * @brief 获取堆当前元素个数
 * @param pq  优先级队列实例指针
 * @return 元素个数；参数为空时返回0
 */
uint32_t ds_pqueue_size(ds_pqueue_t *pq);

/**
 * @brief 判断堆是否为空
 * @param pq  优先级队列实例指针
 * @return 1=空，0=非空
 */
int ds_pqueue_is_empty(ds_pqueue_t *pq);

/**
 * @brief 判断堆是否已满
 * @param pq  优先级队列实例指针
 * @return 1=满，0=未满
 */
int ds_pqueue_is_full(ds_pqueue_t *pq);

#ifdef __cplusplus
}
#endif

#endif /* DS_PRIORITY_QUEUE_H */
