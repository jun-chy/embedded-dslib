/**
 * @file    ds_linkedlist.c
 * @brief   双向链表实现
 * @version 1.0.0
 * @date    2026-05-30
 */

#include "inc/ds_linkedlist.h"

/* ========================================================================
 * API函数实现 - 链表操作
 * ======================================================================== */

ds_err_t ds_list_init(ds_list_t *list)
{
#if DS_ENABLE_CHECK
    if (list == NULL) {
        return DS_ERR_NULL;
    }
#endif

    list->head.prev = &list->head;
    list->head.next = &list->head;
    list->head.data = NULL;
    list->size = 0;
    return DS_OK;
}

ds_err_t ds_list_clear(ds_list_t *list)
{
#if DS_ENABLE_CHECK
    if (list == NULL) {
        return DS_ERR_NULL;
    }
#endif

    list->head.prev = &list->head;
    list->head.next = &list->head;
    list->size = 0;
    return DS_OK;
}

uint32_t ds_list_size(ds_list_t *list)
{
    if (list == NULL) {
        return 0;
    }
    return list->size;
}

int ds_list_is_empty(ds_list_t *list)
{
    if (list == NULL) {
        return 1;
    }
    return (list->size == 0);
}

/* ========================================================================
 * API函数实现 - 节点操作
 * ======================================================================== */

ds_err_t ds_list_node_bind(ds_list_node_t *node, void *data)
{
#if DS_ENABLE_CHECK
    if (node == NULL) {
        return DS_ERR_NULL;
    }
#endif

    node->data = data;
    return DS_OK;
}

void *ds_list_node_data(ds_list_node_t *node)
{
    if (node == NULL) {
        return NULL;
    }
    return node->data;
}

/* ========================================================================
 * API函数实现 - 插入操作
 * ======================================================================== */

ds_err_t ds_list_push_front(ds_list_t *list, ds_list_node_t *node)
{
#if DS_ENABLE_CHECK
    if (list == NULL || node == NULL) {
        return DS_ERR_NULL;
    }
#endif

    /* 在哨兵头节点之后插入 */
    node->next = list->head.next;
    node->prev = &list->head;
    list->head.next->prev = node;
    list->head.next = node;
    list->size++;
    return DS_OK;
}

ds_err_t ds_list_push_back(ds_list_t *list, ds_list_node_t *node)
{
#if DS_ENABLE_CHECK
    if (list == NULL || node == NULL) {
        return DS_ERR_NULL;
    }
#endif

    /* 在哨兵头节点之前插入（即链表尾部） */
    node->prev = list->head.prev;
    node->next = &list->head;
    list->head.prev->next = node;
    list->head.prev = node;
    list->size++;
    return DS_OK;
}

ds_err_t ds_list_insert_before(ds_list_t *list, ds_list_node_t *pos,
                                ds_list_node_t *new_node)
{
#if DS_ENABLE_CHECK
    if (list == NULL || pos == NULL || new_node == NULL) {
        return DS_ERR_NULL;
    }
#endif

    new_node->prev = pos->prev;
    new_node->next = pos;
    pos->prev->next = new_node;
    pos->prev = new_node;
    list->size++;
    return DS_OK;
}

ds_err_t ds_list_insert_after(ds_list_t *list, ds_list_node_t *pos,
                               ds_list_node_t *new_node)
{
#if DS_ENABLE_CHECK
    if (list == NULL || pos == NULL || new_node == NULL) {
        return DS_ERR_NULL;
    }
#endif

    new_node->next = pos->next;
    new_node->prev = pos;
    pos->next->prev = new_node;
    pos->next = new_node;
    list->size++;
    return DS_OK;
}

/* ========================================================================
 * API函数实现 - 删除操作
 * ======================================================================== */

ds_list_node_t *ds_list_pop_front(ds_list_t *list)
{
#if DS_ENABLE_CHECK
    if (list == NULL) {
        return NULL;
    }
#endif

    if (ds_list_is_empty(list)) {
        return NULL;
    }

    ds_list_node_t *node = list->head.next;
    node->next->prev = &list->head;
    list->head.next = node->next;
    list->size--;
    return node;
}

ds_list_node_t *ds_list_pop_back(ds_list_t *list)
{
#if DS_ENABLE_CHECK
    if (list == NULL) {
        return NULL;
    }
#endif

    if (ds_list_is_empty(list)) {
        return NULL;
    }

    ds_list_node_t *node = list->head.prev;
    node->prev->next = &list->head;
    list->head.prev = node->prev;
    list->size--;
    return node;
}

ds_err_t ds_list_remove(ds_list_t *list, ds_list_node_t *node)
{
#if DS_ENABLE_CHECK
    if (list == NULL || node == NULL) {
        return DS_ERR_NULL;
    }
#endif

    node->prev->next = node->next;
    node->next->prev = node->prev;
    list->size--;
    return DS_OK;
}

/* ========================================================================
 * API函数实现 - 查询操作
 * ======================================================================== */

ds_list_node_t *ds_list_front(ds_list_t *list)
{
    if (list == NULL || ds_list_is_empty(list)) {
        return NULL;
    }
    return list->head.next;
}

ds_list_node_t *ds_list_back(ds_list_t *list)
{
    if (list == NULL || ds_list_is_empty(list)) {
        return NULL;
    }
    return list->head.prev;
}

ds_list_node_t *ds_list_at(ds_list_t *list, uint32_t index)
{
#if DS_ENABLE_CHECK
    if (list == NULL) {
        return NULL;
    }
#endif

    if (index >= list->size) {
        return NULL;
    }

    ds_list_node_t *node = list->head.next;
    for (uint32_t i = 0; i < index; i++) {
        node = node->next;
    }
    return node;
}

/* ========================================================================
 * API函数实现 - 遍历与反转
 * ======================================================================== */

ds_err_t ds_list_foreach(ds_list_t *list, ds_list_callback_t callback, void *arg)
{
#if DS_ENABLE_CHECK
    if (list == NULL || callback == NULL) {
        return DS_ERR_NULL;
    }
#endif

    ds_list_node_t *node = list->head.next;
    while (node != &list->head) {
        if (callback(node, arg) != 0) {
            break;
        }
        node = node->next;
    }
    return DS_OK;
}

ds_err_t ds_list_foreach_reverse(ds_list_t *list, ds_list_callback_t callback,
                                  void *arg)
{
#if DS_ENABLE_CHECK
    if (list == NULL || callback == NULL) {
        return DS_ERR_NULL;
    }
#endif

    ds_list_node_t *node = list->head.prev;
    while (node != &list->head) {
        if (callback(node, arg) != 0) {
            break;
        }
        node = node->prev;
    }
    return DS_OK;
}

ds_err_t ds_list_reverse(ds_list_t *list)
{
#if DS_ENABLE_CHECK
    if (list == NULL) {
        return DS_ERR_NULL;
    }
#endif

    if (list->size <= 1) {
        return DS_OK;
    }

    /* 就地反转：交换每个节点的prev和next指针 */
    ds_list_node_t *current = &list->head;
    do {
        ds_list_node_t *temp = current->next;
        current->next = current->prev;
        current->prev = temp;
        current = temp;
    } while (current != &list->head);

    return DS_OK;
}
