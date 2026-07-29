/**
 * @file    uart.c
 * @brief   UART多串口发送驱动实现
 * @details 通过端口号查表获取对应UART外设实例，统一发送逻辑。
 *          printf重定向：AC6编译器通过fputc()，GCC通过_write()，
 *          目标端口均为UART_PORT_1（UART1, TX: PA8）。
 * @note    发送'\n'时自动前置发送'\r'，确保串口终端正确换行。
 *          若更换printf目标串口，修改PRINTF_PORT宏即可。
 */

#include "uart.h"
#include "ti_msp_dl_config.h"

/** printf重定向的目标串口，修改此宏即可切换printf输出端口 */
#define PRINTF_PORT  UART_PORT_1

/**
 * @brief  端口号 → UART外设实例映射表
 * @note   下标与uart_port_t枚举值一一对应，
 *         内容为SysConfig生成的UART_x_INST宏。
 */
static UART_Regs *const uart_inst_map[UART_PORT_MAX] = {
    [UART_PORT_0] = UART_0_INST,
    [UART_PORT_1] = UART_1_INST,
    [UART_PORT_2] = UART_2_INST,
    [UART_PORT_3] = UART_3_INST,
};

/**
  * @brief    发送单个字节
  * @param    port    ：目标串口
  * @param    data    ：待发送的 8 位数据
  * @retval   无
  */
void uart_send_byte(uart_port_t port, uint8_t data) {
    if (port >= UART_PORT_MAX) {
        return;
    }
    DL_UART_Main_transmitData(uart_inst_map[port], data);
    while (DL_UART_Main_isBusy(uart_inst_map[port]));
}

/**
  * @brief    发送字符串（以 '\0' 结尾）
  * @param    port    ：目标串口
  * @param    str     ：待发送的字符串指针（NULL 则直接返回）
  * @retval   无
  */
void uart_send_string(uart_port_t port, const char *str) {
    if (str == NULL) {
        return;
    }
    while (*str) {
        uart_send_byte(port, (uint8_t)*str++);
    }
}

/**
  * @brief    发送指定长度的字节缓冲区
  * @param    port    ：目标串口
  * @param    buf     ：待发送数据缓冲区指针（NULL 则直接返回）
  * @param    len     ：发送字节数
  *                    = 0 : 直接返回，不发送
  * @retval   无
  */
void uart_send_buf(uart_port_t port, const uint8_t *buf, size_t len) {
    if (buf == NULL || len == 0) {
        return;
    }
    for (size_t i = 0; i < len; i++) {
        uart_send_byte(port, buf[i]);
    }
}

/**
  * @brief    阻塞接收单个字节
  * @param    port    ：目标串口
  * @param    data    ：接收数据存放指针（NULL 则返回 -1）
  * @retval    0  : 接收成功
  * @retval   -1  : 参数无效（端口越界或指针为空）
  */
int uart_receive_byte(uart_port_t port, uint8_t *data) {
    if (port >= UART_PORT_MAX || data == NULL) {
        return -1;
    }
    while (DL_UART_Main_isRXFIFOEmpty(uart_inst_map[port]));
    *data = DL_UART_Main_receiveData(uart_inst_map[port]);
    return 0;
}

/**
 * @brief  printf重定向底层接口（AC6: fputc / GCC: _write）
 * @details ARM Compiler 6 通过 fputc 作为 stdout 最终出口；
 *          GCC newlib-nano 通过 _write 系统调用实现。根据编译器宏自动选择。
 *          发送'\n'前自动补发'\r'以兼容常见串口终端。
 */
#if defined(__ARMCC_VERSION)
/* ---- ARM Compiler 6 (AC6) ---- */
int fputc(int ch, FILE *f) {
    if (ch == '\n') {
        uart_send_byte(PRINTF_PORT, '\r');
    }
    uart_send_byte(PRINTF_PORT, (uint8_t)ch);
    return ch;
}
#elif defined(__GNUC__)
/* ---- GCC (newlib-nano) ---- */
int _write(int fd, const char *buf, int len) {
    (void)fd;
    for (int i = 0; i < len; i++) {
        if (buf[i] == '\n') {
            uart_send_byte(PRINTF_PORT, '\r');
        }
        uart_send_byte(PRINTF_PORT, (uint8_t)buf[i]);
    }
    return len;
}
#endif