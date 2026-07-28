/**
 * @file    encoder.h
 * @brief   编码器驱动头文件（正交编码器脉冲计数）
 * @note    encoder_left_count / encoder_right_count 在 GPIO 中断中实时累加，
 *          由 start.c 中 TIMG0 ISR（10ms）定时读取并清零；
 *          encoder_left_speed / encoder_right_speed 为清零时刻的脉冲数（脉冲/10ms）。
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

/** 左轮编码器累计脉冲数（GPIO ISR 中实时更新） */
extern volatile int32_t encoder_left_count;

/** 右轮编码器累计脉冲数（GPIO ISR 中实时更新） */
extern volatile int32_t encoder_right_count;

/** 左轮转速（脉冲/10ms，TIMG0 ISR 中每 10ms 更新） */
extern volatile int32_t encoder_left_speed;

/** 右轮转速（脉冲/10ms，TIMG0 ISR 中每 10ms 更新） */
extern volatile int32_t encoder_right_speed;

/**
  * @brief    编码器初始化（使能 NVIC 中断）
  * @note     GPIO 引脚与中断极性已由 SysConfig（SYSCFG_DL_GPIO_init）配置完毕，
  *           本函数仅打开 NVIC 侧开关，并注册 GROUP1_IRQHandler。
  *           调用前需先执行 SYSCFG_DL_init()。
  * @retval   无
  */
void encoder_init(void);

#endif /* ENCODER_H */
