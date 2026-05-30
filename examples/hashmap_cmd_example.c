/**
 * @file    hashmap_cmd_example.c
 * @brief   哈希表命令解析示例
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 本示例演示如何使用哈希表实现高效的命令解析器：
 * - 将命令字符串映射到对应的处理函数
 * - 支持命令帮助信息查询
 * - 支持动态注册和注销命令
 * - O(1)平均查找时间，适合大量命令的场景
 *
 * 典型应用场景：
 * - CLI命令行界面
 * - AT指令解析
 * - 调试命令处理
 * - 协议命令分发
 */

#include "../inc/ds_hashmap.h"
#include <stdio.h>
#include <string.h>

/* ========================================================================
 * 命令系统配置
 * ======================================================================== */

#define MAX_COMMANDS     32     /* 最大命令数量 */
#define CMD_ARGC_MAX     8      /* 命令参数最大个数 */
#define CMD_LINE_SIZE    128    /* 命令行最大长度 */

/* ========================================================================
 * 类型定义
 * ======================================================================== */

/** 命令处理回调函数 */
typedef void (*cmd_handler_t)(int argc, char *argv[]);

/** 命令条目结构体 */
typedef struct {
    cmd_handler_t handler;     /**< 命令处理函数 */
    const char *help;           /**< 帮助信息 */
} cmd_entry_t;

/* ========================================================================
 * 全局变量
 * ======================================================================== */

/** 命令哈希表 */
static ds_hm_entry_t g_cmd_entries[MAX_COMMANDS];
static ds_hashmap_t g_cmd_map;

/* ========================================================================
 * 内置命令实现
 * ======================================================================== */

/** help命令：显示所有可用命令 */
static void cmd_help(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    printf("Available commands:\n");

    ds_hashmap_foreach(&g_cmd_map,
        (ds_hm_callback_t)(void *)1, NULL);

    /* 手动遍历显示 */
    printf("  help    - Show this help message\n");
    printf("  led     - Control LED (on/off/status)\n");
    printf("  adc     - Read ADC channel\n");
    printf("  reset   - Reset statistics\n");
    printf("  version - Show firmware version\n");
}

/** led命令：控制LED */
static void cmd_led(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: led <on|off|status>\n");
        return;
    }

    if (strcmp(argv[1], "on") == 0) {
        printf("LED turned ON\n");
        /* HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); */
    } else if (strcmp(argv[1], "off") == 0) {
        printf("LED turned OFF\n");
        /* HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); */
    } else if (strcmp(argv[1], "status") == 0) {
        printf("LED status: ON\n");
    } else {
        printf("Unknown argument: %s\n", argv[1]);
    }
}

/** adc命令：读取ADC通道 */
static void cmd_adc(int argc, char *argv[])
{
    int channel = 0;
    if (argc >= 2) {
        channel = atoi(argv[1]);
    }

    /* 模拟ADC读取值 */
    uint16_t adc_value = 2048 + channel * 100;
    float voltage = (float)adc_value * 3.3f / 4095.0f;

    printf("ADC Channel %d: raw=%u, voltage=%.2fV\n",
           channel, adc_value, voltage);
}

/** reset命令：重置统计信息 */
static void cmd_reset(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    printf("Statistics reset\n");
}

/** version命令：显示版本信息 */
static void cmd_version(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    printf("Firmware Version: 1.0.0\n");
    printf("Build Date: 2026-05-30\n");
    printf("DSLib Version: 1.0.0\n");
}

/* ========================================================================
 * 命令注册函数
 * ======================================================================== */

/** 命令条目存储数组 */
static cmd_entry_t g_cmd_store[MAX_COMMANDS];

/**
 * @brief 注册一个命令到哈希表
 */
static ds_err_t register_command(const char *name, cmd_handler_t handler,
                                  const char *help)
{
    /* 检查是否已注册 */
    if (ds_hashmap_contains(&g_cmd_map, name)) {
        printf("[WARN] Command '%s' already registered\n", name);
        return DS_ERR_EXISTS;
    }

    /* 找到空闲条目 */
    for (int i = 0; i < MAX_COMMANDS; i++) {
        if (g_cmd_store[i].handler == NULL) {
            g_cmd_store[i].handler = handler;
            g_cmd_store[i].help = help;
            return ds_hashmap_put(&g_cmd_map, name, &g_cmd_store[i]);
        }
    }
    return DS_ERR_FULL;
}

/* ========================================================================
 * 命令执行器
 * ======================================================================== */

/**
 * @brief 解析并执行命令行
 * @param line  命令行字符串（以'\0'结尾）
 */
static void execute_command(char *line)
{
    char *argv[CMD_ARGC_MAX];
    int argc = 0;

    /* 简单的分词：按空格分割 */
    char *token = strtok(line, " \t\r\n");
    while (token != NULL && argc < CMD_ARGC_MAX) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\r\n");
    }

    if (argc == 0) {
        return;  /* 空行 */
    }

    /* 在哈希表中查找命令 */
    cmd_entry_t *entry = (cmd_entry_t *)ds_hashmap_get(&g_cmd_map, argv[0]);
    if (entry != NULL && entry->handler != NULL) {
        entry->handler(argc, argv);
    } else {
        printf("Unknown command: %s (type 'help' for available commands)\n",
               argv[0]);
    }
}

/* ========================================================================
 * 示例主函数
 * ======================================================================== */

int main(void)
{
    /* 1. 初始化命令哈希表 */
    ds_hashmap_init(&g_cmd_map, g_cmd_entries, MAX_COMMANDS);

    /* 2. 注册内置命令 */
    register_command("help",    cmd_help,    "Show help");
    register_command("led",     cmd_led,     "Control LED");
    register_command("adc",     cmd_adc,     "Read ADC");
    register_command("reset",   cmd_reset,   "Reset stats");
    register_command("version", cmd_version, "Show version");

    printf("=== Command Parser Example ===\n");
    printf("Registered %u commands\n\n", ds_hashmap_size(&g_cmd_map));

    /* 3. 模拟执行命令 */
    char cmd1[] = "version";
    printf("> %s\n", cmd1);
    execute_command(cmd1);

    char cmd2[] = "led on";
    printf("> %s\n", cmd2);
    execute_command(cmd2);

    char cmd3[] = "adc 0";
    printf("> %s\n", cmd3);
    execute_command(cmd3);

    char cmd4[] = "adc 3";
    printf("> %s\n", cmd4);
    execute_command(cmd4);

    char cmd5[] = "unknown";
    printf("> %s\n", cmd5);
    execute_command(cmd5);

    char cmd6[] = "led status";
    printf("> %s\n", cmd6);
    execute_command(cmd6);

    return 0;
}
