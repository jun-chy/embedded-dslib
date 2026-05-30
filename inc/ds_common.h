/**
 * @file    ds_common.h
 * @brief   嵌入式轻量级数据结构库 - 公共类型定义与配置宏
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 本文件定义了所有数据结构共用的类型、错误码和配置宏。
 * 通过配置宏可切换静态分配（零malloc）和动态分配模式。
 */

#ifndef DS_COMMON_H
#define DS_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* ========================================================================
 * 配置宏 - 用户可根据目标平台在编译时定义
 * ======================================================================== */

/**
 * @brief 是否使用动态内存分配（malloc/free）
 * - 默认为 0：纯静态分配，零malloc，适合裸机/RTOS嵌入式环境
 * - 设为 1：允许使用malloc/free，适合有MMU的资源丰富环境
 */
#ifndef DS_USE_MALLOC
#define DS_USE_MALLOC 0
#endif

/**
 * @brief 是否启用参数校验
 * - 默认为 1：进行NULL指针等基本校验
 * - 设为 0：跳过校验以获得极致性能（仅用于已验证的场景）
 */
#ifndef DS_ENABLE_CHECK
#define DS_ENABLE_CHECK 1
#endif

/**
 * @brief 最大支持的数据结构实例数量（用于静态池化分配）
 */
#ifndef DS_MAX_INSTANCES
#define DS_MAX_INSTANCES 4
#endif

/* ========================================================================
 * 错误码定义
 * ======================================================================== */

/**
 * @brief 数据结构库错误码类型
 *
 * 所有API函数返回此类型，便于调用者统一处理错误。
 * 设计原则：零为成功，正数为警告，负数为错误。
 */
typedef enum {
    DS_OK           =  0,   /**< 成功 */
    DS_ERR_NULL     = -1,   /**< 空指针错误：传入的指针为NULL */
    DS_ERR_FULL     = -2,   /**< 容量已满：无法再插入新元素 */
    DS_ERR_EMPTY    = -3,   /**< 容器为空：无法再取出元素 */
    DS_ERR_NOTFOUND = -4,   /**< 未找到：查找操作无匹配结果 */
    DS_ERR_INVALID  = -5,   /**< 参数无效：传入的参数不合法 */
    DS_ERR_NOMEM    = -6,   /**< 内存不足：malloc失败（仅DS_USE_MALLOC=1时） */
    DS_ERR_EXISTS   = -7,   /**< 已存在：重复插入或重复注册 */
    DS_ERR_BUSY     = -8,   /**< 资源忙：定时器/事件正在使用中 */
    DS_ERR_OVERFLOW = -9,   /**< 溢出：数据量超出预设边界 */
    DS_ERR_TIMEOUT  = -10,  /**< 超时：等待时间已过 */
    DS_ERR_INTERNAL = -99,  /**< 内部错误：不应发生的逻辑错误 */
} ds_err_t;

/* ========================================================================
 * 通用工具宏
 * ======================================================================== */

/** 返回数组元素个数（编译期常量） */
#define DS_ARRAY_SIZE(arr)    (sizeof(arr) / sizeof((arr)[0]))

/** 取较小值 */
#define DS_MIN(a, b)          ((a) < (b) ? (a) : (b))

/** 取较大值 */
#define DS_MAX(a, b)          ((a) > (b) ? (a) : (b))

/** 判断是否为2的幂 */
#define DS_IS_POWER_OF_TWO(x) (((x) & ((x) - 1)) == 0)

/** 向上对齐到2的幂 */
#define DS_ALIGN_POWER_OF_TWO(x)                       \
    do {                                                \
        (x)--;                                          \
        (x) |= (x) >> 1;                               \
        (x) |= (x) >> 2;                               \
        (x) |= (x) >> 4;                               \
        (x) |= (x) >> 8;                               \
        (x) |= (x) >> 16;                              \
        (x)++;                                          \
    } while (0)

/** 置零结构体（用于初始化） */
#define DS_ZERO(var)           memset(&(var), 0, sizeof(var))

/** 标记未使用参数，消除编译器警告 */
#define DS_UNUSED(var)        (void)(var)

/* ========================================================================
 * 内联辅助函数
 * ======================================================================== */

/**
 * @brief DJB2字符串哈希函数
 * @param str  输入字符串（需以'\0'结尾）
 * @return 32位哈希值
 *
 * 经典DJB2哈希算法，分布均匀、冲突率低，常用于哈希表。
 */
static inline uint32_t ds_hash_djb2(const char *str)
{
    uint32_t hash = 5381;
    int c;
    while ((c = (unsigned char)*str++) != 0) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

/**
 * @brief 快速幂取模（用于哈希表的二次探测）
 * @param base  底数
 * @param exp   指数
 * @param mod   模数
 * @return (base^exp) % mod
 */
static inline uint32_t ds_pow_mod(uint32_t base, uint32_t exp, uint32_t mod)
{
    uint32_t result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % mod;
        }
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result;
}

#ifdef __cplusplus
}
#endif

#endif /* DS_COMMON_H */
