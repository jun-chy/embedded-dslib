/**
 * @file    ds_ringbuf.h
 * @brief   嵌入式轻量级数据结构库 - 环形缓冲区
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 高性能环形缓冲区实现，专为嵌入式场景设计：
 * - 缓冲区大小必须为2的幂，利用位掩码替代取模运算
 * - 支持单字节和批量读写操作
 * - 适用于UART DMA接收、音频缓冲、日志队列等场景
 *
 * 使用示例：
 * @code
 *   uint8_t buffer[256];  // 必须为2的幂大小
 *   ds_ringbuf_t rb;
 *   ds_ringbuf_init(&rb, buffer, 256);
 *   ds_ringbuf_write(&rb, 'A');
 *   uint8_t ch;
 *   ds_ringbuf_read(&rb, &ch);
 * @endcode
 */

#ifndef DS_RINGBUF_H
#define DS_RINGBUF_H

#include "ds_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 类型定义
 * ======================================================================== */

/**
 * @brief 环形缓冲区实例结构体
 *
 * 采用头尾指针方案，不浪费任何存储空间。
 * 当 head == tail 时缓冲区为空；
 * 当 (head - tail) == capacity 时缓冲区为满。
 */
typedef struct {
    uint8_t *buffer;      /**< 数据缓冲区指针（指向外部内存） */
    uint32_t capacity;    /**< 缓冲区容量（必须为2的幂） */
    uint32_t mask;        /**< 容量掩码 = capacity - 1，用于快速取模 */
    uint32_t head;        /**< 写入位置（下一个写入的索引） */
    uint32_t tail;        /**< 读取位置（下一个读取的索引） */
} ds_ringbuf_t;

/* ========================================================================
 * API函数
 * ======================================================================== */

/**
 * @brief 初始化环形缓冲区
 * @param rb        环形缓冲区实例指针
 * @param buffer    外部提供的缓冲区（大小必须为2的幂）
 * @param capacity  缓冲区容量（必须为2的幂，且 >= 2）
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_INVALID 容量非法
 *
 * @note 此函数不会分配内存，仅绑定已有缓冲区。
 */
ds_err_t ds_ringbuf_init(ds_ringbuf_t *rb, uint8_t *buffer, uint32_t capacity);

/**
 * @brief 重置环形缓冲区（清空所有数据）
 * @param rb  环形缓冲区实例指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_ringbuf_reset(ds_ringbuf_t *rb);

/**
 * @brief 向环形缓冲区写入单个字节
 * @param rb   环形缓冲区实例指针
 * @param data 要写入的数据
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_FULL 缓冲区已满
 */
ds_err_t ds_ringbuf_write(ds_ringbuf_t *rb, uint8_t data);

/**
 * @brief 从环形缓冲区读取单个字节
 * @param rb    环形缓冲区实例指针
 * @param data  读取数据存放地址
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_EMPTY 缓冲区为空
 */
ds_err_t ds_ringbuf_read(ds_ringbuf_t *rb, uint8_t *data);

/**
 * @brief 批量写入数据到环形缓冲区
 * @param rb     环形缓冲区实例指针
 * @param data   待写入数据数组
 * @param len    待写入数据长度（字节）
 * @param written 实际写入的字节数（可为NULL）
 * @return DS_OK 成功写入部分/全部数据；DS_ERR_NULL 参数为空
 *
 * @note 当缓冲区空间不足时，会尽可能多地写入数据，并通过written返回实际写入量。
 */
ds_err_t ds_ringbuf_write_batch(ds_ringbuf_t *rb, const uint8_t *data,
                                 uint32_t len, uint32_t *written);

/**
 * @brief 批量读取数据
 * @param rb       环形缓冲区实例指针
 * @param buf      接收缓冲区
 * @param buf_size 接收缓冲区大小
 * @param read     实际读取的字节数（可为NULL）
 * @return DS_OK 成功读取部分/全部数据；DS_ERR_NULL 参数为空
 */
ds_err_t ds_ringbuf_read_batch(ds_ringbuf_t *rb, uint8_t *buf,
                                uint32_t buf_size, uint32_t *read);

/**
 * @brief 查看但不移除缓冲区头部数据（peek操作）
 * @param rb    环形缓冲区实例指针
 * @param data  查看数据存放地址
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_EMPTY 缓冲区为空
 */
ds_err_t ds_ringbuf_peek(ds_ringbuf_t *rb, uint8_t *data);

/**
 * @brief 获取缓冲区中当前数据量
 * @param rb  环形缓冲区实例指针
 * @return 缓冲区中的字节数；参数为空时返回0
 */
uint32_t ds_ringbuf_available(ds_ringbuf_t *rb);

/**
 * @brief 获取缓冲区剩余可写入空间
 * @param rb  环形缓冲区实例指针
 * @return 可用空间字节数；参数为空时返回0
 */
uint32_t ds_ringbuf_free_space(ds_ringbuf_t *rb);

/**
 * @brief 判断缓冲区是否为空
 * @param rb  环形缓冲区实例指针
 * @return 1=空，0=非空
 */
int ds_ringbuf_is_empty(ds_ringbuf_t *rb);

/**
 * @brief 判断缓冲区是否已满
 * @param rb  环形缓冲区实例指针
 * @return 1=满，0=未满
 */
int ds_ringbuf_is_full(ds_ringbuf_t *rb);

#ifdef __cplusplus
}
#endif

#endif /* DS_RINGBUF_H */
