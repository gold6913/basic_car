/**
 * @file    delay.c
 * @brief   毫秒级延时函数实现（基于 CPU 周期计数）
 * @note    依赖 CPU 主频宏 CPU_CLOCK_HZ，调用 DL_Delay 底层的 delay_cycles
 */

#include "delay.h"
#include "ti_msp_dl_config.h"

/** CPU 主频（Hz），用于 delay_cycles 换算；修改此处适配不同时钟配置 */
#define CPU_CLOCK_HZ  32000000U

/**
  * @brief    毫秒级延时
  * @param    ms  ：延时毫秒数
  * @retval   无
  */
void delay_ms(uint32_t ms)
{
    delay_cycles(ms * (CPU_CLOCK_HZ / 1000U));
}
