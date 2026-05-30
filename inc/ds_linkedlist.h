/**
 * @file    ds_linkedlist.h
 * @brief   嵌入式轻量级数据结构库 - 双向链表
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 通用双向链表实现，带头哨兵节点：
 * - 头节点不存储数据，简化边界处理
 * - 支持前向/反向遍历
 * - 支持任意位置插入/删除
 * - 支持链表反转
 * - 适用于动态任务队列、消息链等场景
 *
 * 使用示例：
 * @code
 *   ds_list_t list;
 *   ds_list_init(&list);
 *   ds_list_node_t nodes[10];
 *   int data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
 *   for (int i = 0; i < 10; i++) {
 *       ds_list_node_bind(&nodes[i], &data[i]);
 *       ds_list_push_back(&list, &nodes[i]);
 *   }
 *   // 遍历输出...
 * @endcode
 */

#ifndef DS_LINKEDLIST_H
#define DS_LINKEDLIST_H

#include "ds_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 类型定义
 * ======================================================================== */

/**
 * @brief 双向链表节点结构体
 *
 * 节点通过 data 指针绑定用户数据，不拷贝数据本身。
 * 节点可嵌入用户结构体中使用，避免额外内存分配。
 */
typedef struct ds_list_node {
    struct ds_list_node *prev;  /**< 前驱节点指针 */
    struct ds_list_node *next;  /**< 后继节点指针 */
    void *data;                 /**< 用户数据指针 */
} ds_list_node_t;

/**
 * @brief 双向链表结构体
 *
 * 包含哨兵头节点，list.head.next 指向第一个有效节点，
 * list.head.prev 指向最后一个有效节点。
 */
typedef struct {
    ds_list_node_t head;         /**< 哨兵头节点（不存储数据） */
    uint32_t size;               /**< 当前链表元素个数 */
} ds_list_t;

/**
 * @brief 链表遍历回调函数类型
 * @param node   当前节点
 * @param arg    用户自定义参数
 * @return 0 继续遍历，非0 停止遍历
 */
typedef int (*ds_list_callback_t)(ds_list_node_t *node, void *arg);

/* ========================================================================
 * API函数 - 链表操作
 * ======================================================================== */

/**
 * @brief 初始化链表
 * @param list  链表实例指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_list_init(ds_list_t *list);

/**
 * @brief 清空链表（移除所有节点，但不释放节点内存）
 * @param list  链表实例指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 *
 * @note 仅解除链表中的链接关系，节点的生命周期由用户管理。
 */
ds_err_t ds_list_clear(ds_list_t *list);

/**
 * @brief 获取链表元素个数
 * @param list  链表实例指针
 * @return 元素个数；参数为空时返回0
 */
uint32_t ds_list_size(ds_list_t *list);

/**
 * @brief 判断链表是否为空
 * @param list  链表实例指针
 * @return 1=空，0=非空
 */
int ds_list_is_empty(ds_list_t *list);

/* ========================================================================
 * API函数 - 节点操作
 * ======================================================================== */

/**
 * @brief 将用户数据绑定到链表节点
 * @param node  节点指针
 * @param data  用户数据指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_list_node_bind(ds_list_node_t *node, void *data);

/**
 * @brief 获取节点绑定的用户数据
 * @param node  节点指针
 * @return 用户数据指针；参数为空时返回NULL
 */
void *ds_list_node_data(ds_list_node_t *node);

/* ========================================================================
 * API函数 - 插入操作
 * ======================================================================== */

/**
 * @brief 在链表头部插入节点
 * @param list  链表实例指针
 * @param node  待插入节点
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_list_push_front(ds_list_t *list, ds_list_node_t *node);

/**
 * @brief 在链表尾部插入节点
 * @param list  链表实例指针
 * @param node  待插入节点
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_list_push_back(ds_list_t *list, ds_list_node_t *node);

/**
 * @brief 在指定节点之前插入新节点
 * @param list      链表实例指针
 * @param pos       位置参考节点
 * @param new_node  待插入节点
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_list_insert_before(ds_list_t *list, ds_list_node_t *pos,
                                ds_list_node_t *new_node);

/**
 * @brief 在指定节点之后插入新节点
 * @param list      链表实例指针
 * @param pos       位置参考节点
 * @param new_node  待插入节点
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_list_insert_after(ds_list_t *list, ds_list_node_t *pos,
                               ds_list_node_t *new_node);

/* ========================================================================
 * API函数 - 删除操作
 * ======================================================================== */

/**
 * @brief 移除链表头部节点
 * @param list  链表实例指针
 * @return 被移除的节点指针；链表为空或参数非法时返回NULL
 */
ds_list_node_t *ds_list_pop_front(ds_list_t *list);

/**
 * @brief 移除链表尾部节点
 * @param list  链表实例指针
 * @return 被移除的节点指针；链表为空或参数非法时返回NULL
 */
ds_list_node_t *ds_list_pop_back(ds_list_t *list);

/**
 * @brief 从链表中移除指定节点
 * @param list  链表实例指针
 * @param node  待移除节点
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_list_remove(ds_list_t *list, ds_list_node_t *node);

/* ========================================================================
 * API函数 - 查询操作
 * ======================================================================== */

/**
 * @brief 获取链表头部节点（不移除）
 * @param list  链表实例指针
 * @return 头部节点指针；链表为空或参数非法时返回NULL
 */
ds_list_node_t *ds_list_front(ds_list_t *list);

/**
 * @brief 获取链表尾部节点（不移除）
 * @param list  链表实例指针
 * @return 尾部节点指针；链表为空或参数非法时返回NULL
 */
ds_list_node_t *ds_list_back(ds_list_t *list);

/**
 * @brief 按索引获取节点（0-based）
 * @param list  链表实例指针
 * @param index 索引值
 * @return 对应节点指针；索引越界或参数非法时返回NULL
 */
ds_list_node_t *ds_list_at(ds_list_t *list, uint32_t index);

/* ========================================================================
 * API函数 - 遍历与反转
 * ======================================================================== */

/**
 * @brief 前向遍历链表
 * @param list      链表实例指针
 * @param callback  回调函数，返回非0则停止遍历
 * @param arg       传递给回调函数的用户参数
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_list_foreach(ds_list_t *list, ds_list_callback_t callback, void *arg);

/**
 * @brief 反向遍历链表（从尾到头）
 * @param list      链表实例指针
 * @param callback  回调函数
 * @param arg       传递给回调函数的用户参数
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_list_foreach_reverse(ds_list_t *list, ds_list_callback_t callback,
                                  void *arg);

/**
 * @brief 反转链表（就地反转，O(n)时间）
 * @param list  链表实例指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_list_reverse(ds_list_t *list);

#ifdef __cplusplus
}
#endif

#endif /* DS_LINKEDLIST_H */
