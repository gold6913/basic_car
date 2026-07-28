/**
 * @file    uart.h
 * @brief   UART多串口发送驱动（BSP层）
 * @details 封装MSPM0G3507的4路UART发送功能：
 *          - UART_PORT_0 (UART0, TX: PA10)  ← printf重定向默认端口
 *          - UART_PORT_1 (UART1, TX: PA8)
 *          - UART_PORT_2 (UART2, TX: PB17)
 *          - UART_PORT_3 (UART3, TX: PB12)
 * @note    所有UART外设初始化由SysConfig生成的SYSCFG_DL_init()完成，
 *          本模块仅提供发送接口，无需额外初始化。
 */

#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

/**
 * @brief 串口号枚举
 * @note  与SysConfig中UART_0~UART_3的配置一一对应
 */
typedef enum
{
    UART_PORT_0 = 0, /**< UART0 (TX: PA10, RX: PA11) - printf默认端口 */
    UART_PORT_1, /**< UART1 (TX: PA8,  RX: PA9) */
    UART_PORT_2, /**< UART2 (TX: PB17, RX: PB16) */
    UART_PORT_3, /**< UART3 (TX: PB12, RX: PB13) */
    UART_PORT_MAX /**< 串口数量上限（不可用作端口号） */
} uart_port_t;

/**
 * @brief  向指定串口发送单字节数据（阻塞等待发送完成）
 * @param  port 串口号，取值 uart_port_t 枚举
 * @param  data 待发送的字节
 * @note   函数会阻塞直到UART发送移位寄存器空闲，
 *         若port超出范围则直接忽略不发送。
 */
void uart_send_byte(uart_port_t port, uint8_t data);

/**
 * @brief  向指定串口发送以'\0'结尾的字符串
 * @param  port 串口号，取值 uart_port_t 枚举
 * @param  str  待发送的字符串指针，不可为NULL
 * @note   内部逐字节调用uart_send_byte发送，不会自动追加'\r'。
 */
void uart_send_string(uart_port_t port, const char *str);

/**
 * @brief  向指定串口发送指定长度的数据缓冲区
 * @param  port 串口号，取值 uart_port_t 枚举
 * @param  buf  待发送的数据缓冲区指针，不可为NULL
 * @param  len  待发送的数据长度（字节数）
 * @note   适用于发送二进制数据或非'\0'结尾的字符串，
 *         若buf为NULL或len为0则直接返回。
 */
void uart_send_buf(uart_port_t port, const uint8_t *buf, size_t len);

/**
 * @brief  从指定串口接收单字节数据（阻塞等待接收完成）
 * @param  port 串口号，取值 uart_port_t 枚举
 * @param  data 接收数据存放指针
 * @retval 0 成功, -1 端口号无效
 * @note   函数会阻塞直到RX FIFO中有数据可读。
 */
int uart_receive_byte(uart_port_t port, uint8_t *data);

#endif