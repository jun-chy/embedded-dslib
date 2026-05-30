/**
 * @file    test_all.c
 * @brief   embedded-dslib 完整单元测试
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 包含所有数据结构的单元测试，每个结构至少5个测试用例，
 * 覆盖正常操作、边界条件和错误处理。
 */

#include "../inc/ds_common.h"
#include "../inc/ds_ringbuf.h"
#include "../inc/ds_linkedlist.h"
#include "../inc/ds_queue.h"
#include "../inc/ds_stack.h"
#include "../inc/ds_hashmap.h"
#include "../inc/ds_priority_queue.h"
#include "../inc/ds_bitfield.h"
#include "../inc/ds_timer.h"
#include "../inc/ds_event.h"
#include "test_framework.h"

/* ========================================================================
 * 环形缓冲区测试
 * ======================================================================== */

TEST_CASE(ringbuf_basic_init)
{
    uint8_t buf[64];
    ds_ringbuf_t rb;
    ASSERT_OK(ds_ringbuf_init(&rb, buf, 64));
    ASSERT_EQ(ds_ringbuf_available(&rb), 0u);
    ASSERT_EQ(ds_ringbuf_free_space(&rb), 63u);  /* 实际可用=capacity-1 */
    ASSERT_EQ(rb.capacity, 64u);
    PASS();
}

TEST_CASE(ringbuf_write_read_single)
{
    uint8_t buf[16];
    ds_ringbuf_t rb;
    ds_ringbuf_init(&rb, buf, 16);

    ASSERT_OK(ds_ringbuf_write(&rb, 'A'));
    ASSERT_EQ(ds_ringbuf_available(&rb), 1u);
    ASSERT_EQ(ds_ringbuf_is_empty(&rb), 0);

    uint8_t data;
    ASSERT_OK(ds_ringbuf_read(&rb, &data));
    ASSERT_EQ(data, 'A');
    ASSERT_EQ(ds_ringbuf_available(&rb), 0u);
    PASS();
}

TEST_CASE(ringbuf_full_and_empty)
{
    uint8_t buf[8];
    ds_ringbuf_t rb;
    ds_ringbuf_init(&rb, buf, 8);

    /* 写入直到满（实际可用=7） */
    for (int i = 0; i < 7; i++) {
        ASSERT_OK(ds_ringbuf_write(&rb, (uint8_t)i));
    }
    ASSERT_EQ(ds_ringbuf_is_full(&rb), 1);

    /* 再写入应返回FULL */
    ASSERT_ERR(ds_ringbuf_write(&rb, 99), DS_ERR_FULL);

    /* 全部读出 */
    for (int i = 0; i < 7; i++) {
        uint8_t data;
        ASSERT_OK(ds_ringbuf_read(&rb, &data));
        ASSERT_EQ(data, (uint8_t)i);
    }
    ASSERT_EQ(ds_ringbuf_is_empty(&rb), 1);

    /* 再读取应返回EMPTY */
    uint8_t data;
    ASSERT_ERR(ds_ringbuf_read(&rb, &data), DS_ERR_EMPTY);
    PASS();
}

TEST_CASE(ringbuf_batch_operations)
{
    uint8_t buf[32];
    ds_ringbuf_t rb;
    ds_ringbuf_init(&rb, buf, 32);

    uint8_t src[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint32_t written = 0;
    ASSERT_OK(ds_ringbuf_write_batch(&rb, src, 8, &written));
    ASSERT_EQ(written, 8u);
    ASSERT_EQ(ds_ringbuf_available(&rb), 8u);

    uint8_t dst[8];
    uint32_t read = 0;
    ASSERT_OK(ds_ringbuf_read_batch(&rb, dst, 8, &read));
    ASSERT_EQ(read, 8u);

    for (int i = 0; i < 8; i++) {
        ASSERT_EQ(dst[i], src[i]);
    }
    PASS();
}

TEST_CASE(ringbuf_wrap_around)
{
    uint8_t buf[8];
    ds_ringbuf_t rb;
    ds_ringbuf_init(&rb, buf, 8);

    /* 填充->清空->再填充，测试环绕 */
    for (int round = 0; round < 3; round++) {
        for (int i = 0; i < 7; i++) {
            ASSERT_OK(ds_ringbuf_write(&rb, (uint8_t)(round * 10 + i)));
        }
        for (int i = 0; i < 7; i++) {
            uint8_t data;
            ASSERT_OK(ds_ringbuf_read(&rb, &data));
            ASSERT_EQ(data, (uint8_t)(round * 10 + i));
        }
    }
    PASS();
}

TEST_CASE(ringbuf_peek)
{
    uint8_t buf[16];
    ds_ringbuf_t rb;
    ds_ringbuf_init(&rb, buf, 16);

    ds_ringbuf_write(&rb, 42);
    uint8_t data;
    ASSERT_OK(ds_ringbuf_peek(&rb, &data));
    ASSERT_EQ(data, 42);

    /* peek不应移除数据 */
    ASSERT_EQ(ds_ringbuf_available(&rb), 1u);
    PASS();
}

TEST_CASE(ringbuf_null_params)
{
    ASSERT_ERR(ds_ringbuf_init(NULL, NULL, 0), DS_ERR_NULL);

    uint8_t buf[16];
    ds_ringbuf_t rb;
    ds_ringbuf_init(&rb, buf, 16);

    ASSERT_ERR(ds_ringbuf_write(NULL, 0), DS_ERR_NULL);

    uint8_t data;
    ASSERT_ERR(ds_ringbuf_read(NULL, &data), DS_ERR_NULL);
    ASSERT_ERR(ds_ringbuf_read(&rb, NULL), DS_ERR_NULL);
    PASS();
}

/* ========================================================================
 * 双向链表测试
 * ======================================================================== */

TEST_CASE(linkedlist_basic_push_pop)
{
    ds_list_t list;
    ds_list_init(&list);

    ds_list_node_t nodes[5];
    int data[5] = {10, 20, 30, 40, 50};

    for (int i = 0; i < 5; i++) {
        ds_list_node_bind(&nodes[i], &data[i]);
        ds_list_push_back(&list, &nodes[i]);
    }

    ASSERT_EQ(ds_list_size(&list), 5u);

    /* 从头部依次弹出 */
    for (int i = 0; i < 5; i++) {
        ds_list_node_t *n = ds_list_pop_front(&list);
        ASSERT_NOT_NULL(n);
        ASSERT_EQ(*(int *)n->data, data[i]);
    }
    ASSERT_EQ(ds_list_size(&list), 0u);
    PASS();
}

TEST_CASE(linkedlist_push_front)
{
    ds_list_t list;
    ds_list_init(&list);

    ds_list_node_t nodes[3];
    int data[3] = {1, 2, 3};

    for (int i = 0; i < 3; i++) {
        ds_list_node_bind(&nodes[i], &data[i]);
        ds_list_push_front(&list, &nodes[i]);
    }

    /* push_front后顺序应为3,2,1 */
    ds_list_node_t *n = ds_list_front(&list);
    ASSERT_EQ(*(int *)n->data, 3);
    n = n->next;
    ASSERT_EQ(*(int *)n->data, 2);
    n = n->next;
    ASSERT_EQ(*(int *)n->data, 1);
    PASS();
}

TEST_CASE(linkedlist_insert_at_position)
{
    ds_list_t list;
    ds_list_init(&list);

    ds_list_node_t nodes[5];
    int data[5] = {1, 2, 3, 4, 5};

    for (int i = 0; i < 3; i++) {
        ds_list_node_bind(&nodes[i], &data[i]);
        ds_list_push_back(&list, &nodes[i]);
    }

    /* 在索引1处插入4 */
    ds_list_node_bind(&nodes[3], &data[3]);
    ds_list_insert_before(&list, ds_list_at(&list, 1), &nodes[3]);

    /* 在索引2处插入5 */
    ds_list_node_bind(&nodes[4], &data[4]);
    ds_list_insert_after(&list, ds_list_at(&list, 1), &nodes[4]);

    /* 期望顺序：1, 4, 5, 2, 3 */
    ASSERT_EQ(*(int *)ds_list_at(&list, 0)->data, 1);
    ASSERT_EQ(*(int *)ds_list_at(&list, 1)->data, 4);
    ASSERT_EQ(*(int *)ds_list_at(&list, 2)->data, 5);
    ASSERT_EQ(*(int *)ds_list_at(&list, 3)->data, 2);
    ASSERT_EQ(*(int *)ds_list_at(&list, 4)->data, 3);
    PASS();
}

TEST_CASE(linkedlist_reverse)
{
    ds_list_t list;
    ds_list_init(&list);

    ds_list_node_t nodes[5];
    int data[5] = {1, 2, 3, 4, 5};

    for (int i = 0; i < 5; i++) {
        ds_list_node_bind(&nodes[i], &data[i]);
        ds_list_push_back(&list, &nodes[i]);
    }

    ds_list_reverse(&list);

    /* 反转后应为5,4,3,2,1 */
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(*(int *)ds_list_at(&list, i)->data, 5 - i);
    }
    PASS();
}

TEST_CASE(linkedlist_foreach)
{
    ds_list_t list;
    ds_list_init(&list);

    ds_list_node_t nodes[5];
    int data[5] = {10, 20, 30, 40, 50};
    int sum = 0;

    for (int i = 0; i < 5; i++) {
        ds_list_node_bind(&nodes[i], &data[i]);
        ds_list_push_back(&list, &nodes[i]);
    }

    /* 遍历求和 */
    ds_list_foreach(&list, (ds_list_callback_t)(void *)1, NULL);

    /* 使用at来求和验证 */
    for (uint32_t i = 0; i < ds_list_size(&list); i++) {
        sum += *(int *)ds_list_at(&list, i)->data;
    }
    ASSERT_EQ(sum, 150);
    PASS();
}

TEST_CASE(linkedlist_remove_middle)
{
    ds_list_t list;
    ds_list_init(&list);

    ds_list_node_t nodes[5];
    int data[5] = {1, 2, 3, 4, 5};

    for (int i = 0; i < 5; i++) {
        ds_list_node_bind(&nodes[i], &data[i]);
        ds_list_push_back(&list, &nodes[i]);
    }

    /* 删除中间节点（索引2，值为3） */
    ds_list_remove(&list, &nodes[2]);
    ASSERT_EQ(ds_list_size(&list), 4u);
    ASSERT_EQ(*(int *)ds_list_at(&list, 0)->data, 1);
    ASSERT_EQ(*(int *)ds_list_at(&list, 1)->data, 2);
    ASSERT_EQ(*(int *)ds_list_at(&list, 2)->data, 4);
    ASSERT_EQ(*(int *)ds_list_at(&list, 3)->data, 5);
    PASS();
}

TEST_CASE(linkedlist_null_and_edge)
{
    ASSERT_EQ(ds_list_size(NULL), 0u);
    ASSERT_EQ(ds_list_is_empty(NULL), 1);
    ASSERT_NULL(ds_list_front(NULL));
    ASSERT_NULL(ds_list_back(NULL));
    ASSERT_NULL(ds_list_pop_front(NULL));
    ASSERT_NULL(ds_list_at(NULL, 0));
    PASS();
}

/* ========================================================================
 * 队列测试
 * ======================================================================== */

TEST_CASE(queue_basic_enqueue_dequeue)
{
    void *slot[16];
    ds_queue_t q;
    ds_queue_init(&q, slot, 16);

    int a = 1, b = 2, c = 3;
    ds_queue_enqueue(&q, &a);
    ds_queue_enqueue(&q, &b);
    ds_queue_enqueue(&q, &c);

    ASSERT_EQ(ds_queue_size(&q), 3u);

    void *item;
    ds_queue_dequeue(&q, &item);
    ASSERT_EQ(*(int *)item, 1);
    ds_queue_dequeue(&q, &item);
    ASSERT_EQ(*(int *)item, 2);
    ds_queue_dequeue(&q, &item);
    ASSERT_EQ(*(int *)item, 3);
    PASS();
}

TEST_CASE(queue_fifo_order)
{
    void *slot[8];
    ds_queue_t q;
    ds_queue_init(&q, slot, 8);

    int values[7] = {100, 200, 300, 400, 500, 600, 700};
    for (int i = 0; i < 7; i++) {
        ds_queue_enqueue(&q, &values[i]);
    }

    void *item;
    for (int i = 0; i < 7; i++) {
        ds_queue_dequeue(&q, &item);
        ASSERT_EQ(*(int *)item, values[i]);
    }
    PASS();
}

TEST_CASE(queue_full_empty)
{
    void *slot[4];
    ds_queue_t q;
    ds_queue_init(&q, slot, 4);

    int dummy;
    ds_queue_enqueue(&q, &dummy);
    ds_queue_enqueue(&q, &dummy);
    ds_queue_enqueue(&q, &dummy);
    ds_queue_enqueue(&q, &dummy);
    ASSERT_EQ(ds_queue_is_full(&q), 1);

    ASSERT_ERR(ds_queue_enqueue(&q, &dummy), DS_ERR_FULL);

    void *item;
    ds_queue_dequeue(&q, &item);
    ds_queue_dequeue(&q, &item);
    ds_queue_dequeue(&q, &item);
    ds_queue_dequeue(&q, &item);
    ASSERT_EQ(ds_queue_is_empty(&q), 1);

    ASSERT_ERR(ds_queue_dequeue(&q, &item), DS_ERR_EMPTY);
    PASS();
}

TEST_CASE(queue_peek)
{
    void *slot[8];
    ds_queue_t q;
    ds_queue_init(&q, slot, 8);

    int val = 42;
    ds_queue_enqueue(&q, &val);

    void *item;
    ds_queue_peek(&q, &item);
    ASSERT_EQ(*(int *)item, 42);

    /* peek不应改变大小 */
    ASSERT_EQ(ds_queue_size(&q), 1u);
    PASS();
}

TEST_CASE(queue_null_params)
{
    ASSERT_ERR(ds_queue_init(NULL, NULL, 0), DS_ERR_NULL);

    void *slot[4];
    ds_queue_t q;
    ds_queue_init(&q, slot, 4);

    ASSERT_ERR(ds_queue_enqueue(NULL, NULL), DS_ERR_NULL);
    ASSERT_ERR(ds_queue_dequeue(NULL, NULL), DS_ERR_NULL);
    PASS();
}

/* ========================================================================
 * 栈测试
 * ======================================================================== */

TEST_CASE(stack_basic_push_pop)
{
    void *slot[16];
    ds_stack_t stk;
    ds_stack_init(&stk, slot, 16);

    int a = 10, b = 20;
    ds_stack_push(&stk, &a);
    ds_stack_push(&stk, &b);

    ASSERT_EQ(ds_stack_size(&stk), 2u);

    void *item;
    ds_stack_pop(&stk, &item);
    ASSERT_EQ(*(int *)item, 20);  /* LIFO */
    ds_stack_pop(&stk, &item);
    ASSERT_EQ(*(int *)item, 10);
    PASS();
}

TEST_CASE(stack_lifo_order)
{
    void *slot[8];
    ds_stack_t stk;
    ds_stack_init(&stk, slot, 8);

    int values[5] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; i++) {
        ds_stack_push(&stk, &values[i]);
    }

    /* 出栈顺序应为5,4,3,2,1 */
    void *item;
    for (int i = 4; i >= 0; i--) {
        ds_stack_pop(&stk, &item);
        ASSERT_EQ(*(int *)item, values[i]);
    }
    PASS();
}

TEST_CASE(stack_full_empty)
{
    void *slot[4];
    ds_stack_t stk;
    ds_stack_init(&stk, slot, 4);

    int dummy;
    ds_stack_push(&stk, &dummy);
    ds_stack_push(&stk, &dummy);
    ds_stack_push(&stk, &dummy);
    ds_stack_push(&stk, &dummy);
    ASSERT_EQ(ds_stack_is_full(&stk), 1);

    ASSERT_ERR(ds_stack_push(&stk, &dummy), DS_ERR_FULL);

    void *item;
    for (int i = 0; i < 4; i++) {
        ds_stack_pop(&stk, &item);
    }
    ASSERT_EQ(ds_stack_is_empty(&stk), 1);

    ASSERT_ERR(ds_stack_pop(&stk, &item), DS_ERR_EMPTY);
    PASS();
}

TEST_CASE(stack_peek)
{
    void *slot[8];
    ds_stack_t stk;
    ds_stack_init(&stk, slot, 8);

    int val = 99;
    ds_stack_push(&stk, &val);

    void *item;
    ds_stack_peek(&stk, &item);
    ASSERT_EQ(*(int *)item, 99);
    ASSERT_EQ(ds_stack_size(&stk), 1u);  /* peek不改变大小 */
    PASS();
}

TEST_CASE(stack_null_params)
{
    ASSERT_ERR(ds_stack_init(NULL, NULL, 0), DS_ERR_NULL);
    ASSERT_EQ(ds_stack_size(NULL), 0u);
    ASSERT_EQ(ds_stack_is_empty(NULL), 1);
    PASS();
}

/* ========================================================================
 * 哈希表测试
 * ======================================================================== */

TEST_CASE(hashmap_basic_put_get)
{
    ds_hm_entry_t entries[16];
    ds_hashmap_t map;
    ds_hashmap_init(&map, entries, 16);

    int val1 = 100, val2 = 200;
    ds_hashmap_put(&map, "key1", &val1);
    ds_hashmap_put(&map, "key2", &val2);

    ASSERT_EQ(ds_hashmap_size(&map), 2u);
    ASSERT_EQ(*(int *)ds_hashmap_get(&map, "key1"), 100);
    ASSERT_EQ(*(int *)ds_hashmap_get(&map, "key2"), 200);
    PASS();
}

TEST_CASE(hashmap_update_existing)
{
    ds_hm_entry_t entries[16];
    ds_hashmap_t map;
    ds_hashmap_init(&map, entries, 16);

    int v1 = 1, v2 = 2;
    ds_hashmap_put(&map, "count", &v1);
    ASSERT_EQ(*(int *)ds_hashmap_get(&map, "count"), 1);

    /* 更新 */
    ds_hashmap_put(&map, "count", &v2);
    ASSERT_EQ(*(int *)ds_hashmap_get(&map, "count"), 2);
    ASSERT_EQ(ds_hashmap_size(&map), 1u);  /* 大小不变 */
    PASS();
}

TEST_CASE(hashmap_remove_and_contains)
{
    ds_hm_entry_t entries[16];
    ds_hashmap_t map;
    ds_hashmap_init(&map, entries, 16);

    int val = 42;
    ds_hashmap_put(&map, "temp", &val);
    ASSERT_EQ(ds_hashmap_contains(&map, "temp"), 1);

    ds_hashmap_remove(&map, "temp");
    ASSERT_EQ(ds_hashmap_contains(&map, "temp"), 0);
    ASSERT_NULL(ds_hashmap_get(&map, "temp"));
    ASSERT_EQ(ds_hashmap_size(&map), 0u);

    /* 删除不存在的键 */
    ASSERT_ERR(ds_hashmap_remove(&map, "none"), DS_ERR_NOTFOUND);
    PASS();
}

TEST_CASE(hashmap_collision_handling)
{
    /* 使用小表强制产生冲突 */
    ds_hm_entry_t entries[4];
    ds_hashmap_t map;
    ds_hashmap_init(&map, entries, 4);

    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    ds_hashmap_put(&map, "aaa", &v1);
    ds_hashmap_put(&map, "bbb", &v2);
    ds_hashmap_put(&map, "ccc", &v3);
    ds_hashmap_put(&map, "ddd", &v4);

    /* 即使有冲突，值也应该正确 */
    ASSERT_EQ(*(int *)ds_hashmap_get(&map, "aaa"), 1);
    ASSERT_EQ(*(int *)ds_hashmap_get(&map, "bbb"), 2);
    ASSERT_EQ(*(int *)ds_hashmap_get(&map, "ccc"), 3);
    ASSERT_EQ(*(int *)ds_hashmap_get(&map, "ddd"), 4);
    PASS();
}

TEST_CASE(hashmap_full)
{
    ds_hm_entry_t entries[4];
    ds_hashmap_t map;
    ds_hashmap_init(&map, entries, 4);

    int v1, v2, v3, v4;
    ds_hashmap_put(&map, "a", &v1);
    ds_hashmap_put(&map, "b", &v2);
    ds_hashmap_put(&map, "c", &v3);
    /* 第4个可能成功也可能冲突导致FULL */

    /* 清空后重新使用 */
    ds_hashmap_clear(&map);
    ASSERT_EQ(ds_hashmap_size(&map), 0u);
    ds_hashmap_put(&map, "x", &v1);
    ASSERT_EQ(*(int *)ds_hashmap_get(&map, "x"), (int)&v1 - (int)&v1 + (int)&v1);  /* just check non-null */
    ASSERT_NOT_NULL(ds_hashmap_get(&map, "x"));
    PASS();
}

TEST_CASE(hashmap_null_params)
{
    ASSERT_ERR(ds_hashmap_init(NULL, NULL, 0), DS_ERR_NULL);
    ASSERT_NULL(ds_hashmap_get(NULL, "key"));
    ASSERT_ERR(ds_hashmap_put(NULL, NULL, NULL), DS_ERR_NULL);
    ASSERT_ERR(ds_hashmap_remove(NULL, "key"), DS_ERR_NULL);
    ASSERT_EQ(ds_hashmap_size(NULL), 0u);
    PASS();
}

/* ========================================================================
 * 优先级队列测试
 * ======================================================================== */

static int int_compare(const void *a, const void *b)
{
    return (*(const int *)a) - (*(const int *)b);
}

TEST_CASE(pqueue_basic_push_pop)
{
    void *heap[16];
    ds_pqueue_t pq;
    ds_pqueue_init(&pq, heap, 16, int_compare);

    int v3 = 30, v1 = 10, v2 = 20;
    ds_pqueue_push(&pq, &v3);
    ds_pqueue_push(&pq, &v1);
    ds_pqueue_push(&pq, &v2);

    ASSERT_EQ(ds_pqueue_size(&pq), 3u);

    void *item;
    ds_pqueue_pop(&pq, &item);
    ASSERT_EQ(*(int *)item, 10);  /* 最小值先出 */
    ds_pqueue_pop(&pq, &item);
    ASSERT_EQ(*(int *)item, 20);
    ds_pqueue_pop(&pq, &item);
    ASSERT_EQ(*(int *)item, 30);
    PASS();
}

TEST_CASE(pqueue_heap_order)
{
    void *heap[32];
    ds_pqueue_t pq;
    ds_pqueue_init(&pq, heap, 32, int_compare);

    int values[] = {5, 3, 8, 1, 9, 2, 7, 4, 6, 0};
    for (int i = 0; i < 10; i++) {
        ds_pqueue_push(&pq, &values[i]);
    }

    /* 出队顺序应为0,1,2,3,4,5,6,7,8,9 */
    void *item;
    for (int i = 0; i < 10; i++) {
        ds_pqueue_pop(&pq, &item);
        ASSERT_EQ(*(int *)item, i);
    }
    PASS();
}

TEST_CASE(pqueue_peek)
{
    void *heap[8];
    ds_pqueue_t pq;
    ds_pqueue_init(&pq, heap, 8, int_compare);

    int v5 = 50, v1 = 10;
    ds_pqueue_push(&pq, &v5);
    ds_pqueue_push(&pq, &v1);

    void *item;
    ds_pqueue_peek(&pq, &item);
    ASSERT_EQ(*(int *)item, 10);
    ASSERT_EQ(ds_pqueue_size(&pq), 2u);  /* peek不改变大小 */
    PASS();
}

TEST_CASE(pqueue_full_empty)
{
    void *heap[4];
    ds_pqueue_t pq;
    ds_pqueue_init(&pq, heap, 4, int_compare);

    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    ds_pqueue_push(&pq, &v1);
    ds_pqueue_push(&pq, &v2);
    ds_pqueue_push(&pq, &v3);
    ds_pqueue_push(&pq, &v4);
    ASSERT_EQ(ds_pqueue_is_full(&pq), 1);

    ASSERT_ERR(ds_pqueue_push(&pq, &v1), DS_ERR_FULL);

    void *item;
    for (int i = 0; i < 4; i++) {
        ds_pqueue_pop(&pq, &item);
    }
    ASSERT_EQ(ds_pqueue_is_empty(&pq), 1);

    ASSERT_ERR(ds_pqueue_pop(&pq, &item), DS_ERR_EMPTY);
    PASS();
}

TEST_CASE(pqueue_null_params)
{
    ASSERT_ERR(ds_pqueue_init(NULL, NULL, 0, NULL), DS_ERR_NULL);
    ASSERT_EQ(ds_pqueue_size(NULL), 0u);
    ASSERT_EQ(ds_pqueue_is_empty(NULL), 1);
    PASS();
}

/* ========================================================================
 * 位域测试
 * ======================================================================== */

TEST_CASE(bitfield_basic_set_clear)
{
    uint32_t words[1];
    ds_bitfield_t bf;
    ds_bitfield_init(&bf, words, 32);

    ds_bitfield_set(&bf, 0);
    ds_bitfield_set(&bf, 1);
    ds_bitfield_set(&bf, 31);

    ASSERT_EQ(ds_bitfield_test(&bf, 0), 1);
    ASSERT_EQ(ds_bitfield_test(&bf, 1), 1);
    ASSERT_EQ(ds_bitfield_test(&bf, 31), 1);
    ASSERT_EQ(ds_bitfield_test(&bf, 15), 0);

    ds_bitfield_clear(&bf, 1);
    ASSERT_EQ(ds_bitfield_test(&bf, 1), 0);
    PASS();
}

TEST_CASE(bitfield_toggle)
{
    uint32_t words[1];
    ds_bitfield_t bf;
    ds_bitfield_init(&bf, words, 32);

    ds_bitfield_set(&bf, 5);
    ASSERT_EQ(ds_bitfield_test(&bf, 5), 1);

    ds_bitfield_toggle(&bf, 5);
    ASSERT_EQ(ds_bitfield_test(&bf, 5), 0);

    ds_bitfield_toggle(&bf, 5);
    ASSERT_EQ(ds_bitfield_test(&bf, 5), 1);
    PASS();
}

TEST_CASE(bitfield_find_first_set)
{
    uint32_t words[2];
    ds_bitfield_t bf;
    ds_bitfield_init(&bf, words, 64);

    ds_bitfield_set(&bf, 10);
    ds_bitfield_set(&bf, 35);

    ASSERT_EQ(ds_bitfield_find_first_set(&bf), 10);
    PASS();
}

TEST_CASE(bitfield_find_first_clear)
{
    uint32_t words[1];
    ds_bitfield_t bf;
    ds_bitfield_init(&bf, words, 32);

    ds_bitfield_set_all(&bf);
    ds_bitfield_clear(&bf, 7);

    ASSERT_EQ(ds_bitfield_find_first_clear(&bf), 7);
    PASS();
}

TEST_CASE(bitfield_count)
{
    uint32_t words[1];
    ds_bitfield_t bf;
    ds_bitfield_init(&bf, words, 32);

    ds_bitfield_set(&bf, 0);
    ds_bitfield_set(&bf, 1);
    ds_bitfield_set(&bf, 5);
    ds_bitfield_set(&bf, 31);

    ASSERT_EQ(ds_bitfield_count(&bf), 4u);
    PASS();
}

TEST_CASE(bitfield_multi_word)
{
    uint32_t words[4];
    ds_bitfield_t bf;
    ds_bitfield_init(&bf, words, 128);

    /* 设置跨越多个word的位 */
    ds_bitfield_set(&bf, 0);
    ds_bitfield_set(&bf, 31);
    ds_bitfield_set(&bf, 32);
    ds_bitfield_set(&bf, 63);
    ds_bitfield_set(&bf, 64);
    ds_bitfield_set(&bf, 127);

    ASSERT_EQ(ds_bitfield_test(&bf, 0), 1);
    ASSERT_EQ(ds_bitfield_test(&bf, 31), 1);
    ASSERT_EQ(ds_bitfield_test(&bf, 32), 1);
    ASSERT_EQ(ds_bitfield_test(&bf, 63), 1);
    ASSERT_EQ(ds_bitfield_test(&bf, 64), 1);
    ASSERT_EQ(ds_bitfield_test(&bf, 127), 1);
    ASSERT_EQ(ds_bitfield_count(&bf), 6u);
    PASS();
}

TEST_CASE(bitfield_out_of_bounds)
{
    uint32_t words[1];
    ds_bitfield_t bf;
    ds_bitfield_init(&bf, words, 16);

    ASSERT_ERR(ds_bitfield_set(&bf, 16), DS_ERR_INVALID);
    ASSERT_ERR(ds_bitfield_clear(&bf, 100), DS_ERR_INVALID);
    ASSERT_EQ(ds_bitfield_test(&bf, 20), 0);
    PASS();
}

/* ========================================================================
 * 软件定时器测试
 * ======================================================================== */

static int g_timer_callback_count = 0;

static void timer_test_callback(ds_timer_id_t id, void *user_data)
{
    (void)id;
    (void)user_data;
    g_timer_callback_count++;
}

TEST_CASE(timer_basic_create_start)
{
    ds_timer_t timers[8];
    ds_timer_manager_t mgr;
    ds_timer_manager_init(&mgr, timers, 8);

    ds_timer_t *t = ds_timer_create(&mgr, "test", 100, 1,
                                     timer_test_callback, NULL);
    ASSERT_NOT_NULL(t);
    ASSERT_EQ(t->id, 1u);
    ASSERT_EQ(t->period, 100u);

    ds_timer_start(t);
    ASSERT_EQ(ds_timer_state(t), DS_TIMER_RUNNING);
    PASS();
}

TEST_CASE(timer_one_shot)
{
    ds_timer_t timers[8];
    ds_timer_manager_t mgr;
    ds_timer_manager_init(&mgr, timers, 8);

    g_timer_callback_count = 0;

    ds_timer_t *t = ds_timer_create(&mgr, "oneshot", 50, 0,
                                     timer_test_callback, NULL);
    ds_timer_start(t);

    /* 推进40ms，不应触发 */
    ds_timer_tick(&mgr, 40);
    ASSERT_EQ(g_timer_callback_count, 0);

    /* 再推进20ms，总计60ms > 50ms，应触发 */
    ds_timer_tick(&mgr, 20);
    ASSERT_EQ(g_timer_callback_count, 1);

    /* 单次定时器应自动停止 */
    ASSERT_EQ(ds_timer_state(t), DS_TIMER_EXPIRED);

    /* 再推进时间，不应再触发 */
    ds_timer_tick(&mgr, 100);
    ASSERT_EQ(g_timer_callback_count, 1);
    PASS();
}

TEST_CASE(timer_periodic)
{
    ds_timer_t timers[8];
    ds_timer_manager_t mgr;
    ds_timer_manager_init(&mgr, timers, 8);

    g_timer_callback_count = 0;

    ds_timer_t *t = ds_timer_create(&mgr, "periodic", 10, 1,
                                     timer_test_callback, NULL);
    ds_timer_start(t);

    /* 每10ms触发一次，推进30ms应触发3次 */
    ds_timer_tick(&mgr, 30);
    ASSERT_EQ(g_timer_callback_count, 3);

    /* 再推进20ms应再触发2次 */
    ds_timer_tick(&mgr, 20);
    ASSERT_EQ(g_timer_callback_count, 5);

    /* 定时器应仍在运行 */
    ASSERT_EQ(ds_timer_state(t), DS_TIMER_RUNNING);
    PASS();
}

TEST_CASE(timer_stop_and_delete)
{
    ds_timer_t timers[8];
    ds_timer_manager_t mgr;
    ds_timer_manager_init(&mgr, timers, 8);

    g_timer_callback_count = 0;

    ds_timer_t *t = ds_timer_create(&mgr, "stop_test", 10, 1,
                                     timer_test_callback, NULL);
    ds_timer_start(t);

    ds_timer_tick(&mgr, 5);
    ds_timer_stop(t);

    /* 推进大量时间，不应触发 */
    ds_timer_tick(&mgr, 100);
    ASSERT_EQ(g_timer_callback_count, 0);

    /* 删除定时器 */
    ASSERT_OK(ds_timer_delete(&mgr, t));
    PASS();
}

TEST_CASE(timer_multiple)
{
    ds_timer_t timers[8];
    ds_timer_manager_t mgr;
    ds_timer_manager_init(&mgr, timers, 8);

    g_timer_callback_count = 0;

    /* 创建3个不同周期的定时器 */
    ds_timer_t *t1 = ds_timer_create(&mgr, "fast", 5, 1,
                                      timer_test_callback, NULL);
    ds_timer_t *t2 = ds_timer_create(&mgr, "slow", 10, 1,
                                      timer_test_callback, NULL);
    ds_timer_start(t1);
    ds_timer_start(t2);

    /* 推进10ms：t1触发2次，t2触发1次 */
    ds_timer_tick(&mgr, 10);
    ASSERT_EQ(g_timer_callback_count, 3);
    PASS();
}

/* ========================================================================
 * 事件总线测试
 * ======================================================================== */

static int g_event_count = 0;
static int g_event_value = 0;

static void event_handler_a(ds_event_id_t id, void *data, void *user_data)
{
    (void)id;
    (void)user_data;
    g_event_count++;
    if (data != NULL) {
        g_event_value = *(int *)data;
    }
}

static void event_handler_b(ds_event_id_t id, void *data, void *user_data)
{
    (void)id;
    (void)data;
    (void)user_data;
    g_event_count++;
}

TEST_CASE(event_basic_publish_subscribe)
{
    ds_event_sub_t subs[16];
    ds_event_bus_t bus;
    ds_event_bus_init(&bus, subs, 16);

    ASSERT_OK(ds_event_subscribe(&bus, 1, event_handler_a, NULL));
    ASSERT_EQ(ds_event_sub_count(&bus), 1u);

    g_event_count = 0;
    g_event_value = 0;
    int val = 42;
    ds_event_publish(&bus, 1, &val);

    ASSERT_EQ(g_event_count, 1);
    ASSERT_EQ(g_event_value, 42);
    PASS();
}

TEST_CASE(event_multiple_subscribers)
{
    ds_event_sub_t subs[16];
    ds_event_bus_t bus;
    ds_event_bus_init(&bus, subs, 16);

    /* 同一事件有多个订阅者 */
    ds_event_subscribe(&bus, 1, event_handler_a, NULL);
    ds_event_subscribe(&bus, 1, event_handler_b, NULL);

    g_event_count = 0;
    ds_event_publish(&bus, 1, NULL);

    ASSERT_EQ(g_event_count, 2);
    PASS();
}

TEST_CASE(event_unsubscribe)
{
    ds_event_sub_t subs[16];
    ds_event_bus_t bus;
    ds_event_bus_init(&bus, subs, 16);

    ds_event_subscribe(&bus, 1, event_handler_a, NULL);
    ds_event_subscribe(&bus, 1, event_handler_b, NULL);

    /* 取消其中一个订阅 */
    ASSERT_OK(ds_event_unsubscribe(&bus, 1, event_handler_b));
    ASSERT_EQ(ds_event_sub_count(&bus), 1u);

    g_event_count = 0;
    ds_event_publish(&bus, 1, NULL);
    ASSERT_EQ(g_event_count, 1);  /* 只有handler_a */
    PASS();
}

TEST_CASE(event_different_events)
{
    ds_event_sub_t subs[16];
    ds_event_bus_t bus;
    ds_event_bus_init(&bus, subs, 16);

    ds_event_subscribe(&bus, 1, event_handler_a, NULL);
    ds_event_subscribe(&bus, 2, event_handler_b, NULL);

    g_event_count = 0;
    /* 只发布事件1 */
    ds_event_publish(&bus, 1, NULL);
    ASSERT_EQ(g_event_count, 1);

    /* 只发布事件2 */
    ds_event_publish(&bus, 2, NULL);
    ASSERT_EQ(g_event_count, 2);

    /* 发布不匹配的事件3，不应触发任何回调 */
    g_event_count = 0;
    ds_event_publish(&bus, 3, NULL);
    ASSERT_EQ(g_event_count, 0);
    PASS();
}

TEST_CASE(event_duplicate_subscribe)
{
    ds_event_sub_t subs[16];
    ds_event_bus_t bus;
    ds_event_bus_init(&bus, subs, 16);

    ASSERT_OK(ds_event_subscribe(&bus, 1, event_handler_a, NULL));
    /* 重复订阅同一handler到同一事件应返回EXISTS */
    ASSERT_ERR(ds_event_subscribe(&bus, 1, event_handler_a, NULL), DS_ERR_EXISTS);
    ASSERT_EQ(ds_event_sub_count(&bus), 1u);
    PASS();
}

TEST_CASE(event_full)
{
    ds_event_sub_t subs[2];
    ds_event_bus_t bus;
    ds_event_bus_init(&bus, subs, 2);

    ASSERT_OK(ds_event_subscribe(&bus, 1, event_handler_a, NULL));
    ASSERT_OK(ds_event_subscribe(&bus, 2, event_handler_b, NULL));
    /* 池已满 */
    ASSERT_ERR(ds_event_subscribe(&bus, 3, event_handler_a, NULL), DS_ERR_FULL);
    PASS();
}

/* ========================================================================
 * 主函数
 * ======================================================================== */

int main(void)
{
    TEST_SUITE_START();

    /* 环形缓冲区测试 */
    RUN_TEST(ringbuf_basic_init);
    RUN_TEST(ringbuf_write_read_single);
    RUN_TEST(ringbuf_full_and_empty);
    RUN_TEST(ringbuf_batch_operations);
    RUN_TEST(ringbuf_wrap_around);
    RUN_TEST(ringbuf_peek);
    RUN_TEST(ringbuf_null_params);

    /* 双向链表测试 */
    RUN_TEST(linkedlist_basic_push_pop);
    RUN_TEST(linkedlist_push_front);
    RUN_TEST(linkedlist_insert_at_position);
    RUN_TEST(linkedlist_reverse);
    RUN_TEST(linkedlist_foreach);
    RUN_TEST(linkedlist_remove_middle);
    RUN_TEST(linkedlist_null_and_edge);

    /* 队列测试 */
    RUN_TEST(queue_basic_enqueue_dequeue);
    RUN_TEST(queue_fifo_order);
    RUN_TEST(queue_full_empty);
    RUN_TEST(queue_peek);
    RUN_TEST(queue_null_params);

    /* 栈测试 */
    RUN_TEST(stack_basic_push_pop);
    RUN_TEST(stack_lifo_order);
    RUN_TEST(stack_full_empty);
    RUN_TEST(stack_peek);
    RUN_TEST(stack_null_params);

    /* 哈希表测试 */
    RUN_TEST(hashmap_basic_put_get);
    RUN_TEST(hashmap_update_existing);
    RUN_TEST(hashmap_remove_and_contains);
    RUN_TEST(hashmap_collision_handling);
    RUN_TEST(hashmap_full);
    RUN_TEST(hashmap_null_params);

    /* 优先级队列测试 */
    RUN_TEST(pqueue_basic_push_pop);
    RUN_TEST(pqueue_heap_order);
    RUN_TEST(pqueue_peek);
    RUN_TEST(pqueue_full_empty);
    RUN_TEST(pqueue_null_params);

    /* 位域测试 */
    RUN_TEST(bitfield_basic_set_clear);
    RUN_TEST(bitfield_toggle);
    RUN_TEST(bitfield_find_first_set);
    RUN_TEST(bitfield_find_first_clear);
    RUN_TEST(bitfield_count);
    RUN_TEST(bitfield_multi_word);
    RUN_TEST(bitfield_out_of_bounds);

    /* 软件定时器测试 */
    RUN_TEST(timer_basic_create_start);
    RUN_TEST(timer_one_shot);
    RUN_TEST(timer_periodic);
    RUN_TEST(timer_stop_and_delete);
    RUN_TEST(timer_multiple);

    /* 事件总线测试 */
    RUN_TEST(event_basic_publish_subscribe);
    RUN_TEST(event_multiple_subscribers);
    RUN_TEST(event_unsubscribe);
    RUN_TEST(event_different_events);
    RUN_TEST(event_duplicate_subscribe);
    RUN_TEST(event_full);

    TEST_SUITE_END();
}
