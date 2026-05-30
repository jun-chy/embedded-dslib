/**
 * @file    test_framework.h
 * @brief   轻量级单元测试框架
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 提供简洁的单元测试基础设施：
 * - 测试用例注册宏
 * - PASS/FAIL结果判定
 * - 用例耗时统计
 * - 汇总报告输出
 *
 * 使用示例：
 * @code
 *   TEST_CASE(test_addition) {
 *       int result = 2 + 3;
 *       ASSERT_EQ(result, 5);
 *       PASS();
 *   }
 *
 *   int main(void) {
 *       TEST_SUITE_START();
 *       RUN_TEST(test_addition);
 *       TEST_SUITE_END();
 *       return 0;
 *   }
 * @endcode
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * 测试框架配置
 * ======================================================================== */

/** 输出颜色（Windows/Unix兼容） */
#if defined(_WIN32) || defined(_WIN64)
#define TF_COLOR_RED     ""
#define TF_COLOR_GREEN   ""
#define TF_COLOR_YELLOW  ""
#define TF_COLOR_RESET   ""
#else
#define TF_COLOR_RED     "\033[31m"
#define TF_COLOR_GREEN   "\033[32m"
#define TF_COLOR_YELLOW  "\033[33m"
#define TF_COLOR_RESET   "\033[0m"
#endif

/* ========================================================================
 * 全局状态
 * ======================================================================== */

/** 测试结果统计 */
typedef struct {
    int total;      /**< 总用例数 */
    int passed;     /**< 通过数 */
    int failed;     /**< 失败数 */
    uint64_t total_time_ms; /**< 总耗时（毫秒） */
} tf_stats_t;

static tf_stats_t tf_g_stats = {0, 0, 0, 0};

/* ========================================================================
 * 断言宏
 * ======================================================================== */

/** 断言条件为真 */
#define ASSERT_TRUE(cond)                                        \
    do {                                                         \
        if (!(cond)) {                                           \
            printf("  FAIL: ASSERT_TRUE(%s) at %s:%d\n",        \
                   #cond, __FILE__, __LINE__);                   \
            tf_g_stats.failed++;                                 \
            return;                                              \
        }                                                        \
    } while (0)

/** 断言条件为假 */
#define ASSERT_FALSE(cond)                                       \
    do {                                                         \
        if (cond) {                                              \
            printf("  FAIL: ASSERT_FALSE(%s) at %s:%d\n",       \
                   #cond, __FILE__, __LINE__);                   \
            tf_g_stats.failed++;                                 \
            return;                                              \
        }                                                        \
    } while (0)

/** 断言相等 */
#define ASSERT_EQ(a, b)                                          \
    do {                                                         \
        if ((a) != (b)) {                                        \
            printf("  FAIL: ASSERT_EQ(%s, %s) => (%lld != %lld) at %s:%d\n", \
                   #a, #b, (long long)(a), (long long)(b),      \
                   __FILE__, __LINE__);                           \
            tf_g_stats.failed++;                                 \
            return;                                              \
        }                                                        \
    } while (0)

/** 断言不相等 */
#define ASSERT_NE(a, b)                                          \
    do {                                                         \
        if ((a) == (b)) {                                        \
            printf("  FAIL: ASSERT_NE(%s, %s) => both are %lld at %s:%d\n", \
                   #a, #b, (long long)(a), __FILE__, __LINE__);  \
            tf_g_stats.failed++;                                 \
            return;                                              \
        }                                                        \
    } while (0)

/** 断言大于 */
#define ASSERT_GT(a, b)                                          \
    do {                                                         \
        if (!((a) > (b))) {                                      \
            printf("  FAIL: ASSERT_GT(%s, %s) at %s:%d\n",       \
                   #a, #b, __FILE__, __LINE__);                  \
            tf_g_stats.failed++;                                 \
            return;                                              \
        }                                                        \
    } while (0)

/** 断言小于 */
#define ASSERT_LT(a, b)                                          \
    do {                                                         \
        if (!((a) < (b))) {                                      \
            printf("  FAIL: ASSERT_LT(%s, %s) at %s:%d\n",       \
                   #a, #b, __FILE__, __LINE__);                  \
            tf_g_stats.failed++;                                 \
            return;                                              \
        }                                                        \
    } while (0)

/** 断言指针非NULL */
#define ASSERT_NOT_NULL(ptr)                                    \
    do {                                                         \
        if ((ptr) == NULL) {                                     \
            printf("  FAIL: ASSERT_NOT_NULL(%s) is NULL at %s:%d\n", \
                   #ptr, __FILE__, __LINE__);                    \
            tf_g_stats.failed++;                                 \
            return;                                              \
        }                                                        \
    } while (0)

/** 断言指针为NULL */
#define ASSERT_NULL(ptr)                                         \
    do {                                                         \
        if ((ptr) != NULL) {                                     \
            printf("  FAIL: ASSERT_NULL(%s) is not NULL at %s:%d\n", \
                   #ptr, __FILE__, __LINE__);                    \
            tf_g_stats.failed++;                                 \
            return;                                              \
        }                                                        \
    } while (0)

/** 断言错误码为DS_OK */
#define ASSERT_OK(err)                                           \
    do {                                                         \
        ds_err_t _e = (err);                                     \
        if (_e != DS_OK) {                                       \
            printf("  FAIL: ASSERT_OK(%s) => %d at %s:%d\n",     \
                   #err, (int)_e, __FILE__, __LINE__);            \
            tf_g_stats.failed++;                                 \
            return;                                              \
        }                                                        \
    } while (0)

/** 断言错误码为特定值 */
#define ASSERT_ERR(err, expected)                               \
    do {                                                         \
        ds_err_t _e = (err);                                     \
        ds_err_t _exp = (expected);                              \
        if (_e != _exp) {                                        \
            printf("  FAIL: ASSERT_ERR(%s) => got %d, expected %d at %s:%d\n", \
                   #err, (int)_e, (int)_exp, __FILE__, __LINE__);\
            tf_g_stats.failed++;                                 \
            return;                                              \
        }                                                        \
    } while (0)

/* ========================================================================
 * 测试用例定义与运行
 * ======================================================================== */

/**
 * @brief 定义一个测试用例函数
 */
#define TEST_CASE(name)                                          \
    static void test_##name(void)

/**
 * @brief 标记测试通过（放在测试函数末尾）
 */
#define PASS()                                                   \
    do {                                                         \
        tf_g_stats.passed++;                                     \
        return;                                                  \
    } while (0)

/* ========================================================================
 * 简易计时器（平台适配）
 * ======================================================================== */

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

static uint64_t tf_get_time_ms(void)
{
    return (uint64_t)GetTickCount();
}

#else
#include <time.h>

static uint64_t tf_get_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

#endif

/* ========================================================================
 * 测试套件控制
 * ======================================================================== */

/**
 * @brief 开始测试套件
 */
#define TEST_SUITE_START()                                       \
    do {                                                         \
        tf_g_stats.total = 0;                                    \
        tf_g_stats.passed = 0;                                    \
        tf_g_stats.failed = 0;                                    \
        tf_g_stats.total_time_ms = tf_get_time_ms();            \
        printf("========================================\n");    \
        printf("  embedded-dslib Unit Tests\n");                \
        printf("========================================\n\n");   \
    } while (0)

/**
 * @brief 运行单个测试用例
 */
#define RUN_TEST(name)                                           \
    do {                                                         \
        uint64_t _start = tf_get_time_ms();                      \
        tf_g_stats.total++;                                      \
        printf("[%3d] %-50s ", tf_g_stats.total, #name);         \
        test_##name();                                           \
        /* 如果没FAIL也没PASS，说明测试函数缺少PASS() */        \
        if (tf_g_stats.passed + tf_g_stats.failed < tf_g_stats.total) { \
            printf(TF_COLOR_YELLOW "SKIP" TF_COLOR_RESET "       (missing PASS())\n"); \
            tf_g_stats.total--;                                  \
        }                                                        \
        uint64_t _end = tf_get_time_ms();                        \
        /* PASS已经在统计中加了passed */                          \
    } while (0)

/**
 * @brief 运行测试用例（带耗时显示）
 */
#define RUN_TEST_TIMED(name)                                      \
    do {                                                         \
        uint64_t _start = tf_get_time_ms();                      \
        tf_g_stats.total++;                                      \
        printf("[%3d] %-50s ", tf_g_stats.total, #name);         \
        test_##name();                                           \
        uint64_t _end = tf_get_time_ms();                        \
        uint64_t _dur = _end - _start;                           \
        /* 如果没有FAIL且passed+failed < total，补上 */          \
        if (tf_g_stats.passed + tf_g_stats.failed < tf_g_stats.total) { \
            printf(TF_COLOR_YELLOW "SKIP" TF_COLOR_RESET "       %llums\n", (unsigned long long)_dur); \
            tf_g_stats.total--;                                  \
        }                                                        \
    } while (0)

/**
 * @brief 结束测试套件并打印汇总
 */
#define TEST_SUITE_END()                                         \
    do {                                                         \
        tf_g_stats.total_time_ms = tf_get_time_ms() - tf_g_stats.total_time_ms; \
        printf("\n========================================\n");  \
        printf("  Results: %d/%d passed", tf_g_stats.passed,   \
               tf_g_stats.passed + tf_g_stats.failed);           \
        if (tf_g_stats.failed > 0) {                             \
            printf(", %d failed", tf_g_stats.failed);            \
        }                                                        \
        printf("\n  Total time: %llums",                         \
               (unsigned long long)tf_g_stats.total_time_ms);    \
        printf("\n========================================\n");  \
        return tf_g_stats.failed > 0 ? 1 : 0;                    \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* TEST_FRAMEWORK_H */
