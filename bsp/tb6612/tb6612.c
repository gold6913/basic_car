/**
 * @file    tb6612.c
 * @brief   TB6612 双路电机驱动芯片底层驱动
 * @note    通过 GPIO 控制电机方向，定时器 PWM 控制转速
 *           - A 电机 方向: AN1/AN2 | PWM: 定时器通道 C1
 *           - B 电机 方向: BN1/BN2 | PWM: 定时器通道 C0
 *           方向引脚电平与电机转向关系:
 *             IN1=1, IN2=0 → 正转
 *             IN1=0, IN2=1 → 反转
 *             IN1=0, IN2=0 → 刹车/停止
 */

#include "tb6612.h"

/**********************************************************
*** 电机控制
**********************************************************/

/**
  * @brief    设置双路电机转速与方向
  * @param    speedA  ：A 电机速度值
  *                    > 0 : 正转，速度值为 PWM 占空比
  *                    < 0 : 反转，速度绝对值作为 PWM 占空比
  *                    = 0 : A 电机刹车停止
  * @param    speedB  ：B 电机速度值（同上）
  * @param    move    ：运行模式
  *                    = 0 : 紧急停止（双电机全部刹车）
  *                    = 1 : 正常运动（根据 speedA/speedB 分别控制）
  * @retval   无
  */
void motor_set(float speedA, float speedB, uint8_t move)
{
    /*========================== 模式 0：紧急停止 ==========================*/
    if (move == 0)
    {
        /* A 电机方向引脚全部拉低 → 刹车 */
        DL_GPIO_clearPins(TB6612_AN1_PORT, TB6612_AN1_PIN);
        DL_GPIO_clearPins(TB6612_AN2_PORT, TB6612_AN2_PIN);
        /* B 电机方向引脚全部拉低 → 刹车 */
        DL_GPIO_clearPins(TB6612_BN1_PORT, TB6612_BN1_PIN);
        DL_GPIO_clearPins(TB6612_BN2_PORT, TB6612_BN2_PIN);
    }
    /*========================== 模式 1：正常运动 ==========================*/
    else if (move == 1)
    {
        /*------------------ A 电机控制 ------------------*/
        if (speedA == 0)
        {
            /* 速度为 0 → A 电机刹车 */
            DL_GPIO_clearPins(TB6612_AN1_PORT, TB6612_AN1_PIN);
            DL_GPIO_clearPins(TB6612_AN2_PORT, TB6612_AN2_PIN);
        }
        else if (speedA > 0)
        {
            /* 正转：AN1=1, AN2=0，设置 PWM 占空比 */
            DL_GPIO_setPins(TB6612_AN1_PORT, TB6612_AN1_PIN);
            DL_GPIO_clearPins(TB6612_AN2_PORT, TB6612_AN2_PIN);
            DL_TimerG_setCaptureCompareValue(motor_INST, (uint32_t)speedA, GPIO_motor_C1_IDX);
        }
        else if (speedA < 0)
        {
            /* 反转：AN1=0, AN2=1，取绝对值设置 PWM 占空比 */
            DL_GPIO_clearPins(TB6612_AN1_PORT, TB6612_AN1_PIN);
            DL_GPIO_setPins(TB6612_AN2_PORT, TB6612_AN2_PIN);
            DL_TimerG_setCaptureCompareValue(motor_INST, (uint32_t)(-speedA), GPIO_motor_C1_IDX);
        }

        /*------------------ B 电机控制 ------------------*/
        if (speedB == 0)
        {
            /* 速度为 0 → B 电机刹车 */
            DL_GPIO_clearPins(TB6612_BN1_PORT, TB6612_BN1_PIN);
            DL_GPIO_clearPins(TB6612_BN2_PORT, TB6612_BN2_PIN);
        }
        else if (speedB > 0)
        {
            /* 正转：BN1=0, BN2=1，设置 PWM 占空比 */
            DL_GPIO_clearPins(TB6612_BN1_PORT, TB6612_BN1_PIN);
            DL_GPIO_setPins(TB6612_BN2_PORT, TB6612_BN2_PIN);
            DL_TimerG_setCaptureCompareValue(motor_INST, (uint32_t)speedB, GPIO_motor_C0_IDX);
        }
        else if (speedB < 0)
        {
            /* 反转：BN1=1, BN2=0，取绝对值设置 PWM 占空比 */
            DL_GPIO_setPins(TB6612_BN1_PORT, TB6612_BN1_PIN);
            DL_GPIO_clearPins(TB6612_BN2_PORT, TB6612_BN2_PIN);
            DL_TimerG_setCaptureCompareValue(motor_INST, (uint32_t)(-speedB), GPIO_motor_C0_IDX);
        }
    }
}
