/**
 * @file    ds_bitfield.c
 * @brief   位域操作实现
 * @version 1.0.0
 * @date    2026-05-30
 */

#include "inc/ds_bitfield.h"

#include <string.h>

/* ========================================================================
 * 内部辅助函数
 * ======================================================================== */

/**
 * @brief 计算一个uint32_t中的置位位数（32位popcount）
 */
static uint32_t popcount32(uint32_t v)
{
    v = v - ((v >> 1) & 0x55555555u);
    v = (v & 0x33333333u) + ((v >> 2) & 0x33333333u);
    v = (v + (v >> 4)) & 0x0F0F0F0Fu;
    v = (v * 0x01010101u) >> 24;
    return v;
}

/**
 * @brief 查找uint32_t中最低置位的位索引（类似CTZ）
 * @return 最低置位的索引（0~31）；无置位时返回32
 */
static uint32_t ctz32(uint32_t v)
{
    if (v == 0) return 32;

    /* 使用二分法查找最低置位 */
    uint32_t n = 0;
    if ((v & 0x0000FFFFu) == 0) { n += 16; v >>= 16; }
    if ((v & 0x000000FFu) == 0) { n += 8;  v >>= 8;  }
    if ((v & 0x0000000Fu) == 0) { n += 4;  v >>= 4;  }
    if ((v & 0x00000003u) == 0) { n += 2;  v >>= 2;  }
    if ((v & 0x00000001u) == 0) { n += 1; }
    return n;
}

/**
 * @brief 查找uint32_t中最低清除位的位索引
 * @return 最低清除位的索引（0~31）；全为1时返回32
 */
static uint32_t ctz32_not(uint32_t v)
{
    return ctz32(~v);
}

/* ========================================================================
 * API函数实现
 * ======================================================================== */

ds_err_t ds_bitfield_init(ds_bitfield_t *bf, uint32_t *words, uint32_t num_bits)
{
#if DS_ENABLE_CHECK
    if (bf == NULL || words == NULL) {
        return DS_ERR_NULL;
    }
    if (num_bits == 0) {
        return DS_ERR_INVALID;
    }
#endif

    bf->words     = words;
    bf->num_bits  = num_bits;
    bf->num_words = (num_bits + 31) / 32;
    return DS_OK;
}

ds_err_t ds_bitfield_clear_all(ds_bitfield_t *bf)
{
#if DS_ENABLE_CHECK
    if (bf == NULL) {
        return DS_ERR_NULL;
    }
#endif

    memset(bf->words, 0, bf->num_words * sizeof(uint32_t));
    return DS_OK;
}

ds_err_t ds_bitfield_set_all(ds_bitfield_t *bf)
{
#if DS_ENABLE_CHECK
    if (bf == NULL) {
        return DS_ERR_NULL;
    }
#endif

    memset(bf->words, 0xFF, bf->num_words * sizeof(uint32_t));
    return DS_OK;
}

ds_err_t ds_bitfield_set(ds_bitfield_t *bf, uint32_t bit)
{
#if DS_ENABLE_CHECK
    if (bf == NULL) {
        return DS_ERR_NULL;
    }
    if (bit >= bf->num_bits) {
        return DS_ERR_INVALID;
    }
#endif

    uint32_t word_idx = bit / 32;
    uint32_t bit_idx  = bit % 32;
    bf->words[word_idx] |= (1u << bit_idx);
    return DS_OK;
}

ds_err_t ds_bitfield_clear(ds_bitfield_t *bf, uint32_t bit)
{
#if DS_ENABLE_CHECK
    if (bf == NULL) {
        return DS_ERR_NULL;
    }
    if (bit >= bf->num_bits) {
        return DS_ERR_INVALID;
    }
#endif

    uint32_t word_idx = bit / 32;
    uint32_t bit_idx  = bit % 32;
    bf->words[word_idx] &= ~(1u << bit_idx);
    return DS_OK;
}

ds_err_t ds_bitfield_toggle(ds_bitfield_t *bf, uint32_t bit)
{
#if DS_ENABLE_CHECK
    if (bf == NULL) {
        return DS_ERR_NULL;
    }
    if (bit >= bf->num_bits) {
        return DS_ERR_INVALID;
    }
#endif

    uint32_t word_idx = bit / 32;
    uint32_t bit_idx  = bit % 32;
    bf->words[word_idx] ^= (1u << bit_idx);
    return DS_OK;
}

int ds_bitfield_test(ds_bitfield_t *bf, uint32_t bit)
{
#if DS_ENABLE_CHECK
    if (bf == NULL || bit >= bf->num_bits) {
        return 0;
    }
#endif

    uint32_t word_idx = bit / 32;
    uint32_t bit_idx  = bit % 32;
    return (bf->words[word_idx] & (1u << bit_idx)) ? 1 : 0;
}

int ds_bitfield_find_first_set(ds_bitfield_t *bf)
{
#if DS_ENABLE_CHECK
    if (bf == NULL) {
        return -1;
    }
#endif

    for (uint32_t i = 0; i < bf->num_words; i++) {
        if (bf->words[i] != 0) {
            uint32_t bit = ctz32(bf->words[i]);
            uint32_t result = i * 32 + bit;
            /* 确保不超出总位数 */
            if (result >= bf->num_bits) {
                return -1;
            }
            return (int)result;
        }
    }
    return -1;
}

int ds_bitfield_find_first_clear(ds_bitfield_t *bf)
{
#if DS_ENABLE_CHECK
    if (bf == NULL) {
        return -1;
    }
#endif

    for (uint32_t i = 0; i < bf->num_words; i++) {
        if (bf->words[i] != 0xFFFFFFFFu) {
            uint32_t bit = ctz32_not(bf->words[i]);
            uint32_t result = i * 32 + bit;
            if (result >= bf->num_bits) {
                return -1;
            }
            return (int)result;
        }
    }
    return -1;
}

uint32_t ds_bitfield_count(ds_bitfield_t *bf)
{
#if DS_ENABLE_CHECK
    if (bf == NULL) {
        return 0;
    }
#endif

    uint32_t total = 0;
    for (uint32_t i = 0; i < bf->num_words; i++) {
        total += popcount32(bf->words[i]);
    }

    /* 最后一个word可能有不完整的位需要排除 */
    uint32_t last_word_bits = bf->num_bits % 32;
    if (last_word_bits != 0 && bf->num_words > 0) {
        /* 重新计算最后一个word，只计算有效位 */
        total -= popcount32(bf->words[bf->num_words - 1]);
        uint32_t mask = (1u << last_word_bits) - 1;
        total += popcount32(bf->words[bf->num_words - 1] & mask);
    }

    return total;
}

uint32_t ds_bitfield_size(ds_bitfield_t *bf)
{
    if (bf == NULL) {
        return 0;
    }
    return bf->num_bits;
}
