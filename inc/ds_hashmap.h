/**
 * @file    ds_hashmap.h
 * @brief   嵌入式轻量级数据结构库 - 哈希表
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 基于开放寻址法的固定大小哈希表：
 * - 使用DJB2字符串哈希 + 二次探测解决冲突
 * - 键类型为字符串，值类型为void指针
 * - 所有内存静态预分配，零malloc
 * - 支持键值对的增删改查
 * - 适用于命令解析、配置管理、设备注册表等场景
 *
 * 使用示例：
 * @code
 *   ds_hm_entry_t entries[64];
 *   ds_hashmap_t map;
 *   ds_hashmap_init(&map, entries, 64);
 *   int val = 100;
 *   ds_hashmap_put(&map, "temperature", &val);
 *   int *p = ds_hashmap_get(&map, "temperature");
 *   // *p == 100
 * @endcode
 */

#ifndef DS_HASHMAP_H
#define DS_HASHMAP_H

#include "ds_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 类型定义
 * ======================================================================== */

/** 哈希表槽位最大键长（含'\0'） */
#define DS_HM_MAX_KEY_LEN 32

/** 标记槽位为空的特殊值 */
#define DS_HM_SLOT_EMPTY    0xFFFFFFFFu

/** 标记槽位为已删除的特殊值 */
#define DS_HM_SLOT_DELETED  0xFFFFFFFEu

/**
 * @brief 哈希表条目结构体
 *
 * 每个槽位存储一个键值对。key_hash 用于快速判断槽位是否匹配，
 * 避免每次比较都进行完整的字符串比较。
 */
typedef struct {
    uint32_t key_hash;                /**< 键的哈希值（用于快速比对） */
    char     key[DS_HM_MAX_KEY_LEN];  /**< 键字符串 */
    void    *value;                   /**< 值指针 */
} ds_hm_entry_t;

/**
 * @brief 哈希表实例结构体
 */
typedef struct {
    ds_hm_entry_t *entries;  /**< 条目数组（指向外部内存） */
    uint32_t capacity;       /**< 槽位总数 */
    uint32_t size;           /**< 当前已存储的键值对数量 */
    uint32_t deleted_count;  /**< 已删除的槽位数量（影响探测长度） */
} ds_hashmap_t;

/**
 * @brief 哈希表遍历回调函数类型
 * @param key    当前条目的键
 * @param value  当前条目的值
 * @param arg    用户自定义参数
 * @return 0 继续遍历，非0 停止遍历
 */
typedef int (*ds_hm_callback_t)(const char *key, void *value, void *arg);

/* ========================================================================
 * API函数
 * ======================================================================== */

/**
 * @brief 初始化哈希表
 * @param map       哈希表实例指针
 * @param entries   外部提供的条目数组
 * @param capacity  槽位总数（建议为素数以减少冲突）
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_INVALID 容量为0
 */
ds_err_t ds_hashmap_init(ds_hashmap_t *map, ds_hm_entry_t *entries,
                          uint32_t capacity);

/**
 * @brief 清空哈希表
 * @param map  哈希表实例指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_hashmap_clear(ds_hashmap_t *map);

/**
 * @brief 插入或更新键值对
 * @param map    哈希表实例指针
 * @param key    键字符串（最长DS_HM_MAX_KEY_LEN-1个字符）
 * @param value  值指针
 * @return DS_OK 成功；DS_ERR_NULL 参数为空；DS_ERR_FULL 表已满；DS_ERR_NOMEM 键过长
 */
ds_err_t ds_hashmap_put(ds_hashmap_t *map, const char *key, void *value);

/**
 * @brief 根据键查找值
 * @param map  哈希表实例指针
 * @param key  键字符串
 * @return 值指针；未找到或参数非法时返回NULL
 */
void *ds_hashmap_get(ds_hashmap_t *map, const char *key);

/**
 * @brief 根据键删除条目
 * @param map  哈希表实例指针
 * @param key  键字符串
 * @return DS_OK 成功删除；DS_ERR_NULL 参数为空；DS_ERR_NOTFOUND 键不存在
 */
ds_err_t ds_hashmap_remove(ds_hashmap_t *map, const char *key);

/**
 * @brief 检查键是否存在
 * @param map  哈希表实例指针
 * @param key  键字符串
 * @return 1=存在，0=不存在
 */
int ds_hashmap_contains(ds_hashmap_t *map, const char *key);

/**
 * @brief 获取当前键值对数量
 * @param map  哈希表实例指针
 * @return 键值对数量；参数为空时返回0
 */
uint32_t ds_hashmap_size(ds_hashmap_t *map);

/**
 * @brief 遍历哈希表
 * @param map       哈希表实例指针
 * @param callback  回调函数，返回非0则停止遍历
 * @param arg       传递给回调函数的用户参数
 * @return DS_OK 成功；DS_ERR_NULL 参数为空
 */
ds_err_t ds_hashmap_foreach(ds_hashmap_t *map, ds_hm_callback_t callback,
                              void *arg);

#ifdef __cplusplus
}
#endif

#endif /* DS_HASHMAP_H */
