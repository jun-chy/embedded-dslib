/**
 * @file    ringbuf_uart_example.c
 * @brief   环形缓冲区 + UART DMA 使用示例
 * @version 1.0.0
 * @date    2026-05-30
 *
 * 本示例演示如何使用环形缓冲区配合UART DMA进行高效数据接收：
 * - UART DMA自动将接收数据写入环形缓冲区
 * - 主循环中从环形缓冲区取出数据并处理
 * - 利用位掩码优化的快速读写实现零拷贝数据流
 *
 * 典型应用场景：
 * - 串口通信数据接收
 * - Modbus协议数据帧解析
 * - GPS NMEA数据解析
 * - 蓝牙模块数据接收
 */

#include "../inc/ds_ringbuf.h"
#include <stdio.h>

/* ========================================================================
 * 硬件配置（根据实际MCU修改）
 * ======================================================================== */

#define UART_RX_BUF_SIZE    256    /* UART接收缓冲区大小（必须为2的幂） */
#define LINE_BUF_SIZE       128    /* 行处理缓冲区大小 */

/* ========================================================================
 * 全局变量
 * ======================================================================== */

/** UART接收环形缓冲区 */
static uint8_t g_uart_rx_buffer[UART_RX_BUF_SIZE];
static ds_ringbuf_t g_uart_rx_ringbuf;

/** 数据处理统计 */
static uint32_t g_total_bytes_received = 0;
static uint32_t g_total_lines_processed = 0;

/* ========================================================================
 * 模拟硬件函数（实际项目中替换为真实的HAL函数）
 * ======================================================================== */

/**
 * @brief 模拟UART DMA接收完成中断回调
 *
 * 在实际STM32项目中，这对应 HAL_UART_RxCpltCallback() 或自定义的
 * DMA接收中断处理函数。DMA每次接收完数据后会自动触发此回调。
 *
 * @param data  DMA接收到的数据指针
 * @param len   本次接收的数据长度
 */
void sim_uart_dma_rx_callback(uint8_t *data, uint32_t len)
{
    /* 将DMA接收到的数据写入环形缓冲区 */
    uint32_t written = 0;
    ds_ringbuf_write_batch(&g_uart_rx_ringbuf, data, len, &written);

    if (written < len) {
        printf("[WARN] UART RX buffer overflow! Dropped %u bytes\n",
               len - written);
    }

    g_total_bytes_received += written;

    /* 重启DMA接收（实际项目中调用HAL函数） */
    /* HAL_UART_Receive_DMA(&huart1, dma_buffer, DMA_BUF_SIZE); */
}

/**
 * @brief 模拟主循环处理
 *
 * 从环形缓冲区中取出数据，按行解析处理。
 * 在实际项目中放入 main() 的 while(1) 循环中。
 */
void process_uart_data(void)
{
    uint8_t line_buf[LINE_BUF_SIZE];
    uint32_t line_pos = 0;
    uint8_t ch;

    /* 持续读取直到缓冲区为空 */
    while (ds_ringbuf_read(&g_uart_rx_ringbuf, &ch) == DS_OK) {
        if (ch == '\n' || ch == '\r') {
            if (line_pos > 0) {
                line_buf[line_pos] = '\0';
                g_total_lines_processed++;

                /* TODO: 在此处添加实际的数据解析逻辑 */
                printf("[RX] Line #%u (%u bytes): %s\n",
                       g_total_lines_processed, line_pos, line_buf);

                line_pos = 0;
            }
        } else if (line_pos < LINE_BUF_SIZE - 1) {
            line_buf[line_pos++] = ch;
        }
    }
}

/* ========================================================================
 * 示例主函数
 * ======================================================================== */

int main(void)
{
    /* 1. 初始化UART接收环形缓冲区 */
    ds_err_t err = ds_ringbuf_init(&g_uart_rx_ringbuf,
                                    g_uart_rx_buffer,
                                    UART_RX_BUF_SIZE);
    if (err != DS_OK) {
        printf("[ERROR] Ring buffer init failed: %d\n", err);
        return -1;
    }

    printf("=== UART DMA Ring Buffer Example ===\n");
    printf("Buffer size: %u bytes\n", UART_RX_BUF_SIZE);
    printf("Free space: %u bytes\n", ds_ringbuf_free_space(&g_uart_rx_ringbuf));

    /* 2. 模拟DMA接收数据 */
    uint8_t sim_data[] = "Hello UART DMA!\r\nThis is line 2.\r\n";
    sim_uart_dma_rx_callback(sim_data, sizeof(sim_data) - 1);

    /* 3. 主循环处理数据 */
    printf("\n--- Processing received data ---\n");
    process_uart_data();

    /* 4. 打印统计信息 */
    printf("\n--- Statistics ---\n");
    printf("Total bytes received: %u\n", g_total_bytes_received);
    printf("Total lines processed: %u\n", g_total_lines_processed);
    printf("Buffer remaining: %u bytes\n",
           ds_ringbuf_available(&g_uart_rx_ringbuf));

    return 0;
}
