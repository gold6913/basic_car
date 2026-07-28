/**
 * @file    encoder.c
 * @brief   编码器 GPIO 脉冲计数驱动（正交编码器，四线上升沿）
 * @note    硬件连接（SysConfig 已配）:
 *           - 左编码器: A=PB27, B=PB26  |  右编码器: A=PA24, B=PA25
 *           GPIO 中断（GROUP1）: A/B 四线上升沿 → 实时累加脉冲到全局变量
 *           方向判断:
 *             A 上升沿 + B=0 → 正转(+1)  |  A 上升沿 + B=1 → 反转(-1)
 *             B 上升沿 + A=1 → 正转(+1)  |  B 上升沿 + A=0 → 反转(-1)
 *           转速差分计算与串级 PID 控制由 start.c 的 TIMG0_IRQHandler 负责。
 */

#include "encoder.h"
#include "ti_msp_dl_config.h"

/**********************************************************
*** 全局变量
**********************************************************/

volatile int32_t encoder_left_count  = 0;   /**< 左轮累计脉冲（GROUP1 ISR 更新） */
volatile int32_t encoder_right_count = 0;   /**< 右轮累计脉冲（GROUP1 ISR 更新） */

volatile int32_t encoder_left_speed  = 0;   /**< 左轮转速 脉冲/10ms（TIMG0 ISR 更新） */
volatile int32_t encoder_right_speed = 0;   /**< 右轮转速 脉冲/10ms（TIMG0 ISR 更新） */

/**********************************************************
*** 编码器初始化
**********************************************************/

/**
  * @brief    编码器初始化（使能 GPIO 中断）
  * @note     GPIO 引脚与中断极性已由 SysConfig（SYSCFG_DL_GPIO_init）配置完毕，
  *           本函数仅打开 NVIC 侧 GROUP1 中断开关。
  *           定时器 TIMG0 的启动由 start.c 的 init() 负责。
  *           调用前需先执行 SYSCFG_DL_init()。
  * @retval   无
  */
/**
  * @brief    编码器 GPIO 中断初始化
  * @note     使能左（PB26/PB27）和右（PA24/PA25）两组编码器的 GPIO 中断
  * @retval   无
  */
void encoder_init(void)
{
    /*------------------ GPIO GROUP1 中断：实时脉冲累加 ------------------*/
    NVIC_ClearPendingIRQ(ENCODER_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(ENCODER_GPIOA_INT_IRQN);

    NVIC_ClearPendingIRQ(ENCODER_GPIOB_INT_IRQN);
    NVIC_EnableIRQ(ENCODER_GPIOB_INT_IRQN);
}

/**********************************************************
*** GPIO GROUP1 中断服务函数
**********************************************************/

/**
  * @brief    GPIO GROUP1 中断服务函数
  * @note     处理 GPIOA/GPIOB 上引脚（16-31）所有中断。
  *           编码器方向判断规则:
  *             A 相触发 → B=0 正转, B=1 反转
  *             B 相触发 → A=1 正转, A=0 反转
  * @retval   无
  */
void GROUP1_IRQHandler(void)
{
    /*==================== GPIOA 上引脚 ====================*/
    /*--- 右编码器 A 相 (PA24) ---*/
    if (DL_GPIO_getEnabledInterruptStatus(GPIOA, ENCODER_right_A_PIN)
        & ENCODER_right_A_PIN)
    {
        DL_GPIO_clearInterruptStatus(GPIOA, ENCODER_right_A_PIN);
        /* A 上升沿: B=0 → 正转, B=1 → 反转 */
        if (DL_GPIO_readPins(ENCODER_right_B_PORT, ENCODER_right_B_PIN))
        {
            encoder_right_count--;                       // B=1, 反转
        }
        else
        {
            encoder_right_count++;                       // B=0, 正转
        }
    }

    /*--- 右编码器 B 相 (PA25) ---*/
    if (DL_GPIO_getEnabledInterruptStatus(ENCODER_right_B_PORT, ENCODER_right_B_PIN)
        & ENCODER_right_B_PIN)
    {
        DL_GPIO_clearInterruptStatus(ENCODER_right_B_PORT, ENCODER_right_B_PIN);
        /* B 上升沿: A=1 → 正转, A=0 → 反转 */
        if (DL_GPIO_readPins(ENCODER_right_A_PORT, ENCODER_right_A_PIN))
        {
            encoder_right_count++;                       // A=1, 正转
        }
        else
        {
            encoder_right_count--;                       // A=0, 反转
        }
    }

    /*==================== GPIOB 上引脚 ====================*/
    /*--- 左编码器 A 相 (PB27) ---*/
    if (DL_GPIO_getEnabledInterruptStatus(GPIOB, ENCODER_left_A_PIN)
        & ENCODER_left_A_PIN)
    {
        DL_GPIO_clearInterruptStatus(GPIOB, ENCODER_left_A_PIN);
        /* A 上升沿: B=0 → 正转, B=1 → 反转 */
        if (DL_GPIO_readPins(ENCODER_left_B_PORT, ENCODER_left_B_PIN))
        {
            encoder_left_count++;                        // B=1, 正转
        }
        else
        {
            encoder_left_count--;                        // B=0, 反转
        }
    }

    /*--- 左编码器 B 相 (PB26) ---*/
    if (DL_GPIO_getEnabledInterruptStatus(GPIOB, ENCODER_left_B_PIN)
        & ENCODER_left_B_PIN)
    {
        DL_GPIO_clearInterruptStatus(GPIOB, ENCODER_left_B_PIN);
        /* B 上升沿: A=1 → 反转, A=0 → 正转 */
        if (DL_GPIO_readPins(ENCODER_left_A_PORT, ENCODER_left_A_PIN))
        {
            encoder_left_count--;                        // A=1, 反转
        }
        else
        {
            encoder_left_count++;                        // A=0, 正转
        }
    }
}
