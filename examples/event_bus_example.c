/**
 * @file    event_bus_example.c
 * @brief   事件总线传感器通知示例
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 本示例演示如何使用事件总线实现传感器数据的通知与响应：
 * - 传感器模块发布事件，多个订阅者同时收到通知
 * - 显示模块更新UI
 * - 日志模块记录数据
 * - 告警模块检查阈值
 * - 模块间完全解耦，互不依赖
 *
 * 典型应用场景：
 * - 温湿度传感器数据分发
 * - 按键事件通知
 * - 通信模块消息分发
 * - 系统状态变更广播
 */

#include "../inc/ds_event.h"
#include <stdio.h>
#include <stdint.h>

/* ========================================================================
 * 事件ID定义
 * ======================================================================== */

#define EVT_TEMP_UPDATE      1    /**< 温度数据更新事件 */
#define EVT_HUMIDITY_UPDATE  2    /**< 湿度数据更新事件 */
#define EVT_BUTTON_PRESS     3    /**< 按键按下事件 */
#define EVT_SYSTEM_ALERT     4    /**< 系统告警事件 */

/* ========================================================================
 * 传感器数据类型
 * ======================================================================== */

typedef struct {
    uint8_t sensor_id;    /**< 传感器编号 */
    float value;          /**< 传感器值 */
    uint32_t timestamp;   /**< 时间戳 */
} sensor_data_t;

/* ========================================================================
 * 订阅者模块实现
 * ======================================================================== */

/**
 * @brief 显示模块回调 - 更新LCD显示
 */
static void display_on_temp_update(ds_event_id_t id, void *data, void *arg)
{
    (void)id;
    (void)arg;

    sensor_data_t *sd = (sensor_data_t *)data;
    printf("[DISPLAY] Sensor %u: Temperature = %.1f C\n",
           sd->sensor_id, sd->value);
}

/**
 * @brief 显示模块回调 - 更新湿度显示
 */
static void display_on_humidity_update(ds_event_id_t id, void *data, void *arg)
{
    (void)id;
    (void)arg;

    sensor_data_t *sd = (sensor_data_t *)data;
    printf("[DISPLAY] Sensor %u: Humidity = %.1f %%\n",
           sd->sensor_id, sd->value);
}

/**
 * @brief 日志模块回调 - 记录所有传感器事件
 */
static void logger_on_sensor_event(ds_event_id_t id, void *data, void *arg)
{
    (void)arg;

    const char *evt_name = "unknown";
    switch (id) {
        case EVT_TEMP_UPDATE:     evt_name = "TEMP"; break;
        case EVT_HUMIDITY_UPDATE: evt_name = "HUMIDITY"; break;
        case EVT_BUTTON_PRESS:    evt_name = "BUTTON"; break;
        default: break;
    }

    if (data != NULL) {
        sensor_data_t *sd = (sensor_data_t *)data;
        printf("[LOG] %s event from sensor %u: %.2f (ts=%u)\n",
               evt_name, sd->sensor_id, sd->value, sd->timestamp);
    } else {
        printf("[LOG] %s event (no data)\n", evt_name);
    }
}

/**
 * @brief 告警模块回调 - 检查温度是否超阈值
 */
static void alarm_on_temp_update(ds_event_id_t id, void *data, void *arg)
{
    (void)id;
    float *threshold = (float *)arg;

    sensor_data_t *sd = (sensor_data_t *)data;
    if (sd->value > *threshold) {
        printf("[ALERT] HIGH TEMPERATURE WARNING: %.1f C exceeds threshold %.1f C!\n",
               sd->value, *threshold);
    }
}

/**
 * @brief 按键处理回调
 */
static void input_on_button_press(ds_event_id_t id, void *data, void *arg)
{
    (void)id;
    (void)arg;

    if (data != NULL) {
        uint8_t button_id = *(uint8_t *)data;
        printf("[INPUT] Button %u pressed\n", button_id);
    }
}

/* ========================================================================
 * 示例主函数
 * ======================================================================== */

int main(void)
{
    /* 1. 初始化事件总线 */
    #define MAX_SUBS 16
    ds_event_sub_t subs[MAX_SUBS];
    ds_event_bus_t bus;
    ds_event_bus_init(&bus, subs, MAX_SUBS);

    /* 2. 温度告警阈值 */
    float temp_threshold = 35.0f;

    /* 3. 注册订阅者 */
    /* 温度事件：显示模块 + 日志模块 + 告警模块 */
    ds_event_subscribe(&bus, EVT_TEMP_UPDATE, display_on_temp_update, NULL);
    ds_event_subscribe(&bus, EVT_TEMP_UPDATE, logger_on_sensor_event, NULL);
    ds_event_subscribe(&bus, EVT_TEMP_UPDATE, alarm_on_temp_update,
                      &temp_threshold);

    /* 湿度事件：显示模块 + 日志模块 */
    ds_event_subscribe(&bus, EVT_HUMIDITY_UPDATE, display_on_humidity_update, NULL);
    ds_event_subscribe(&bus, EVT_HUMIDITY_UPDATE, logger_on_sensor_event, NULL);

    /* 按键事件：输入模块 + 日志模块 */
    ds_event_subscribe(&bus, EVT_BUTTON_PRESS, input_on_button_press, NULL);
    ds_event_subscribe(&bus, EVT_BUTTON_PRESS, logger_on_sensor_event, NULL);

    printf("=== Event Bus Sensor Example ===\n");
    printf("Active subscribers: %u\n\n", ds_event_sub_count(&bus));

    /* 4. 模拟传感器事件 */
    printf("--- Simulating sensor events ---\n\n");

    /* 温度事件 1：正常温度 */
    sensor_data_t temp1 = {.sensor_id = 1, .value = 25.3f, .timestamp = 1000};
    ds_event_publish(&bus, EVT_TEMP_UPDATE, &temp1);

    printf("\n");

    /* 湿度事件：正常湿度 */
    sensor_data_t hum1 = {.sensor_id = 1, .value = 60.5f, .timestamp = 1001};
    ds_event_publish(&bus, EVT_HUMIDITY_UPDATE, &hum1);

    printf("\n");

    /* 温度事件 2：高温告警 */
    sensor_data_t temp2 = {.sensor_id = 1, .value = 42.8f, .timestamp = 2000};
    ds_event_publish(&bus, EVT_TEMP_UPDATE, &temp2);

    printf("\n");

    /* 按键事件 */
    uint8_t btn = 3;
    ds_event_publish(&bus, EVT_BUTTON_PRESS, &btn);

    printf("\n");

    /* 温度事件 3：恢复正常 */
    sensor_data_t temp3 = {.sensor_id = 2, .value = 22.1f, .timestamp = 3000};
    ds_event_publish(&bus, EVT_TEMP_UPDATE, &temp3);

    return 0;
}
