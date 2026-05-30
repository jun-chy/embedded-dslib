/**
 * @file    ds_hashmap.c
 * @brief   哈希表实现（开放寻址法 + DJB2哈希 + 二次探测）
 * @version 1.0.0
 * @date    2026-05-30
 */

#include "inc/ds_hashmap.h"

#include <string.h>

/* ========================================================================
 * 内部辅助函数
 * ======================================================================== */

/**
 * @brief 判断槽位是否为空
 */
static inline int hm_is_empty(const ds_hm_entry_t *entry)
{
    return (entry->key_hash == DS_HM_SLOT_EMPTY);
}

/**
 * @brief 判断槽位是否为已删除
 */
static inline int hm_is_deleted(const ds_hm_entry_t *entry)
{
    return (entry->key_hash == DS_HM_SLOT_DELETED);
}

/**
 * @brief 判断槽位是否被占用
 */
static inline int hm_is_occupied(const ds_hm_entry_t *entry)
{
    return (entry->key_hash != DS_HM_SLOT_EMPTY &&
            entry->key_hash != DS_HM_SLOT_DELETED);
}

/**
 * @brief 标记槽位为空
 */
static inline void hm_mark_empty(ds_hm_entry_t *entry)
{
    entry->key_hash = DS_HM_SLOT_EMPTY;
    entry->key[0] = '\0';
    entry->value = NULL;
}

/**
 * @brief 标记槽位为已删除
 */
static inline void hm_mark_deleted(ds_hm_entry_t *entry)
{
    entry->key_hash = DS_HM_SLOT_DELETED;
    entry->key[0] = '\0';
    entry->value = NULL;
}

/**
 * @brief 二次探测：计算第i次探测的位置
 * @param hash  初始哈希值
 * @param i     探测次数（从0开始）
 * @param cap   哈希表容量
 * @return 探测位置索引
 *
 * 使用公式：pos = (hash + i * i) % cap
 */
static inline uint32_t hm_probe(uint32_t hash, uint32_t i, uint32_t cap)
{
    return (hash + i * i) % cap;
}

/* ========================================================================
 * API函数实现
 * ======================================================================== */

ds_err_t ds_hashmap_init(ds_hashmap_t *map, ds_hm_entry_t *entries,
                          uint32_t capacity)
{
#if DS_ENABLE_CHECK
    if (map == NULL || entries == NULL) {
        return DS_ERR_NULL;
    }
    if (capacity == 0) {
        return DS_ERR_INVALID;
    }
#endif

    map->entries  = entries;
    map->capacity = capacity;
    map->size     = 0;
    map->deleted_count = 0;

    /* 初始化所有槽位为空 */
    for (uint32_t i = 0; i < capacity; i++) {
        hm_mark_empty(&map->entries[i]);
    }

    return DS_OK;
}

ds_err_t ds_hashmap_clear(ds_hashmap_t *map)
{
#if DS_ENABLE_CHECK
    if (map == NULL) {
        return DS_ERR_NULL;
    }
#endif

    for (uint32_t i = 0; i < map->capacity; i++) {
        hm_mark_empty(&map->entries[i]);
    }
    map->size = 0;
    map->deleted_count = 0;
    return DS_OK;
}

ds_err_t ds_hashmap_put(ds_hashmap_t *map, const char *key, void *value)
{
#if DS_ENABLE_CHECK
    if (map == NULL || key == NULL) {
        return DS_ERR_NULL;
    }
#endif

    /* 检查键长度 */
    if (strlen(key) >= DS_HM_MAX_KEY_LEN) {
        return DS_ERR_NOMEM;
    }

    uint32_t hash = ds_hash_djb2(key);
    uint32_t first_deleted = map->capacity; /* 记录第一个删除槽位 */
    uint32_t cap = map->capacity;

    /* 最多探测 capacity 次 */
    for (uint32_t i = 0; i < cap; i++) {
        uint32_t idx = hm_probe(hash, i, cap);
        ds_hm_entry_t *entry = &map->entries[idx];

        if (hm_is_empty(entry)) {
            /* 找到空槽位，如果之前有删除槽位，优先使用 */
            uint32_t target = (first_deleted < cap) ? first_deleted : idx;

            map->entries[target].key_hash = hash;
            strncpy(map->entries[target].key, key, DS_HM_MAX_KEY_LEN - 1);
            map->entries[target].key[DS_HM_MAX_KEY_LEN - 1] = '\0';
            map->entries[target].value = value;
            map->size++;

            /* 如果使用了之前记录的删除槽位，减少删除计数 */
            if (first_deleted < cap && target == first_deleted) {
                map->deleted_count--;
            }
            return DS_OK;
        }

        if (hm_is_deleted(entry)) {
            /* 记录第一个遇到的删除槽位 */
            if (first_deleted == cap) {
                first_deleted = idx;
            }
            continue;
        }

        /* 槽位已被占用，检查是否是同一个键（更新操作） */
        if (entry->key_hash == hash && strcmp(entry->key, key) == 0) {
            entry->value = value;
            return DS_OK;
        }
    }

    /* 探测完毕仍未找到空位，检查是否有删除槽位可复用 */
    if (first_deleted < cap) {
        map->entries[first_deleted].key_hash = hash;
        strncpy(map->entries[first_deleted].key, key, DS_HM_MAX_KEY_LEN - 1);
        map->entries[first_deleted].key[DS_HM_MAX_KEY_LEN - 1] = '\0';
        map->entries[first_deleted].value = value;
        map->size++;
        map->deleted_count--;
        return DS_OK;
    }

    return DS_ERR_FULL;
}

void *ds_hashmap_get(ds_hashmap_t *map, const char *key)
{
#if DS_ENABLE_CHECK
    if (map == NULL || key == NULL) {
        return NULL;
    }
#endif

    uint32_t hash = ds_hash_djb2(key);
    uint32_t cap = map->capacity;

    for (uint32_t i = 0; i < cap; i++) {
        uint32_t idx = hm_probe(hash, i, cap);
        ds_hm_entry_t *entry = &map->entries[idx];

        if (hm_is_empty(entry)) {
            /* 遇到空槽位，说明键不存在 */
            return NULL;
        }

        if (hm_is_deleted(entry)) {
            continue;
        }

        if (entry->key_hash == hash && strcmp(entry->key, key) == 0) {
            return entry->value;
        }
    }

    return NULL;
}

ds_err_t ds_hashmap_remove(ds_hashmap_t *map, const char *key)
{
#if DS_ENABLE_CHECK
    if (map == NULL || key == NULL) {
        return DS_ERR_NULL;
    }
#endif

    uint32_t hash = ds_hash_djb2(key);
    uint32_t cap = map->capacity;

    for (uint32_t i = 0; i < cap; i++) {
        uint32_t idx = hm_probe(hash, i, cap);
        ds_hm_entry_t *entry = &map->entries[idx];

        if (hm_is_empty(entry)) {
            return DS_ERR_NOTFOUND;
        }

        if (hm_is_deleted(entry)) {
            continue;
        }

        if (entry->key_hash == hash && strcmp(entry->key, key) == 0) {
            hm_mark_deleted(entry);
            map->size--;
            map->deleted_count++;
            return DS_OK;
        }
    }

    return DS_ERR_NOTFOUND;
}

int ds_hashmap_contains(ds_hashmap_t *map, const char *key)
{
    return (ds_hashmap_get(map, key) != NULL) ? 1 : 0;
}

uint32_t ds_hashmap_size(ds_hashmap_t *map)
{
    if (map == NULL) {
        return 0;
    }
    return map->size;
}

ds_err_t ds_hashmap_foreach(ds_hashmap_t *map, ds_hm_callback_t callback,
                              void *arg)
{
#if DS_ENABLE_CHECK
    if (map == NULL || callback == NULL) {
        return DS_ERR_NULL;
    }
#endif

    for (uint32_t i = 0; i < map->capacity; i++) {
        if (hm_is_occupied(&map->entries[i])) {
            if (callback(map->entries[i].key, map->entries[i].value, arg) != 0) {
                break;
            }
        }
    }
    return DS_OK;
}
