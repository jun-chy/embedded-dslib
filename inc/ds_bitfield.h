/**
 * @file    ds_bitfield.h
 * @brief   嵌入式轻量级数据结构库 - 位域操作工具集
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 提供对位数组的原子级操作：
 * - 基于uint32_t数组的位图，支持任意位数
 * - 支持设置、清除、翻转、测试单个比特
 * - 支持批量操作：查找第一个置位/清除位（类似ffs/ffz）
 * - 适用于硬件寄存器管理、资源分配位图、标志位管理等场景
 *
 * 使用示例：
 * @code
 *   uint32_t bits[4];  // 4 * 32 = 128位
 *   ds_bitfield_t bf;
 *   ds_bitfield_init(&bf, bits, 128);
 *   ds_bitfield_set(&bf, 5);       // 设置第5位
 *   ds_bitfield_test(&bf, 5);      // 返回1
 *   ds_bitfield_clear(&bf, 5);      // 清除第5位
 *   int first = ds_bitfield_find_first_set(&bf);  // 查找第一个置位
 * @endcode
 */

#ifndef DS_BITFIELD_H
#define DS_BITFIELD_H

#include "ds_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 类型定义
 * ======================================================================== */

/**
 * @brief 位域实例结构体
 */
typedef struct {
    uint32_t *words;      /**< 位图数组（指向外部uint32_t数组） */
    uint32_t num_bits;    /**< 总位数 */
    uint32_t num_words;   /**< 使用的uint32_t个数 */
} ds_bitfield_t;

/* ========================================================================
 * API函数
 * ======================================================================== */

/**
 * @brief 初始化位域
 * @param bf        位域实例指针
 * @param words     外部提供的uint32_t数组
 * @param num_bits  总位数（>= 1）
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_INVALID 位数为0
 */
ds_err_t ds_bitfield_init(ds_bitfield_t *bf, uint32_t *words, uint32_t num_bits);

/**
 * @brief 清零位域（清除所有位）
 * @param bf  位域实例指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_bitfield_clear_all(ds_bitfield_t *bf);

/**
 * @brief 设置所有位（全部置1）
 * @param bf  位域实例指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_bitfield_set_all(ds_bitfield_t *bf);

/**
 * @brief 设置指定位
 * @param bf   位域实例指针
 * @param bit  位索引（从0开始）
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_INVALID 索引越界
 */
ds_err_t ds_bitfield_set(ds_bitfield_t *bf, uint32_t bit);

/**
 * @brief 清除指定位
 * @param bf   位域实例指针
 * @param bit  位索引（从0开始）
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_INVALID 索引越界
 */
ds_err_t ds_bitfield_clear(ds_bitfield_t *bf, uint32_t bit);

/**
 * @brief 翻转指定位
 * @param bf   位域实例指针
 * @param bit  位索引（从0开始）
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_INVALID 索引越界
 */
ds_err_t ds_bitfield_toggle(ds_bitfield_t *bf, uint32_t bit);

/**
 * @brief 测试指定位是否为1
 * @param bf   位域实例指针
 * @param bit  位索引（从0开始）
 * @return 1=该位为1，0=该位为0；参数非法时返回0
 */
int ds_bitfield_test(ds_bitfield_t *bf, uint32_t bit);

/**
 * @brief 查找第一个置位的位置（从低位开始）
 * @param bf  位域实例指针
 * @return 第一个置位的位索引；全为0时返回-1；参数非法时返回-1
 */
int ds_bitfield_find_first_set(ds_bitfield_t *bf);

/**
 * @brief 查找第一个清除位的位置（从低位开始）
 * @param bf  位域实例指针
 * @return 第一个清除位的位索引；全为1时返回-1；参数非法时返回-1
 */
int ds_bitfield_find_first_clear(ds_bitfield_t *bf);

/**
 * @brief 统计置位位的数量（popcount）
 * @param bf  位域实例指针
 * @return 置位位数；参数非法时返回0
 */
uint32_t ds_bitfield_count(ds_bitfield_t *bf);

/**
 * @brief 获取位域总位数
 * @param bf  位域实例指针
 * @return 总位数；参数为空时返回0
 */
uint32_t ds_bitfield_size(ds_bitfield_t *bf);

#ifdef __cplusplus
}
#endif

#endif /* DS_BITFIELD_H */
