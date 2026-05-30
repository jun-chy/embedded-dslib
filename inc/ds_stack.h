/**
 * @file    ds_stack.h
 * @brief   嵌入式轻量级数据结构库 - 固定大小栈
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 基于数组的后进先出（LIFO）栈实现：
 * - 使用预分配的void指针数组，零动态内存
 * - 适用于表达式求值、状态保存、DFS遍历等场景
 * - 所有操作均为O(1)时间复杂度
 *
 * 使用示例：
 * @code
 *   void *slot[32];
 *   ds_stack_t stk;
 *   ds_stack_init(&stk, slot, 32);
 *   int a = 10, b = 20;
 *   ds_stack_push(&stk, &a);
 *   ds_stack_push(&stk, &b);
 *   int *p = ds_stack_pop(&stk);  // p == &b
 * @endcode
 */

#ifndef DS_STACK_H
#define DS_STACK_H

#include "ds_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 类型定义
 * ======================================================================== */

/**
 * @brief 固定大小栈实例结构体
 *
 * 使用void指针数组存储元素，可容纳任意类型。
 * top 指向下一个可用位置（即栈顶元素的上方）。
 */
typedef struct {
    void **buffer;       /**< 指针缓冲区（指向外部数组） */
    uint32_t capacity;   /**< 栈最大容量 */
    uint32_t top;        /**< 栈顶指针（当前元素个数） */
} ds_stack_t;

/* ========================================================================
 * API函数
 * ======================================================================== */

/**
 * @brief 初始化栈
 * @param stk       栈实例指针
 * @param buffer    外部提供的指针数组
 * @param capacity  栈最大容量
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_INVALID 容量为0
 */
ds_err_t ds_stack_init(ds_stack_t *stk, void **buffer, uint32_t capacity);

/**
 * @brief 重置栈（清空所有元素）
 * @param stk  栈实例指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_stack_reset(ds_stack_t *stk);

/**
 * @brief 压栈操作（将元素压入栈顶）
 * @param stk   栈实例指针
 * @param item  数据指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_FULL 栈已满
 */
ds_err_t ds_stack_push(ds_stack_t *stk, void *item);

/**
 * @brief 出栈操作（从栈顶弹出元素）
 * @param stk   栈实例指针
 * @param item  弹出数据指针存放地址
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_EMPTY 栈为空
 */
ds_err_t ds_stack_pop(ds_stack_t *stk, void **item);

/**
 * @brief 查看栈顶元素（不出栈）
 * @param stk   栈实例指针
 * @param item  数据指针存放地址
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_EMPTY 栈为空
 */
ds_err_t ds_stack_peek(ds_stack_t *stk, void **item);

/**
 * @brief 获取栈当前元素个数
 * @param stk  栈实例指针
 * @return 元素个数；参数为空时返回0
 */
uint32_t ds_stack_size(ds_stack_t *stk);

/**
 * @brief 判断栈是否为空
 * @param stk  栈实例指针
 * @return 1=空，0=非空
 */
int ds_stack_is_empty(ds_stack_t *stk);

/**
 * @brief 判断栈是否已满
 * @param stk  栈实例指针
 * @return 1=满，0=未满
 */
int ds_stack_is_full(ds_stack_t *stk);

#ifdef __cplusplus
}
#endif

#endif /* DS_STACK_H */
