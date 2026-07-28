/**
 * @file    pid.c
 * @brief   PID 控制器实现（位置式，带积分抗饱和与输出限幅）
 * @note    所有参数和目标值均使用 float 类型
 */

#include "pid.h"

/**
  * @brief    数值限幅
  * @param    value  ：输入值
  * @param    min    ：下限
  * @param    max    ：上限
  * @retval   限幅后的值
  */
static float clamp(float value, float min, float max)
{
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

/**
  * @brief    初始化 PID 控制器
  * @param    pid      ：PID 结构体指针
  * @param    kp       ：比例系数
  * @param    ki       ：积分系数
  * @param    kd       ：微分系数
  * @param    out_min  ：输出下限
  * @param    out_max  ：输出上限
  * @retval   无
  */
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd,
              float out_min, float out_max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->target     = 0.0f;
    pid->error      = 0.0f;
    pid->last_error = 0.0f;
    pid->prev_error = 0.0f;
    pid->integral   = 0.0f;

    pid->out_max = out_max;
    pid->out_min = out_min;

    /* 积分限幅默认与输出限幅一致，防止积分饱和 */
    pid->integral_max = out_max;
    pid->integral_min = out_min;

    pid->output = 0.0f;
}

/**
  * @brief    设定 PID 目标值
  * @param    pid    ：PID 结构体指针
  * @param    target ：目标值
  * @retval   无
  */
void PID_SetTarget(PID_TypeDef *pid, float target)
{
    pid->target = target;
}

/**
  * @brief    在线修改 PID 参数（不重置内部状态）
  * @param    pid    ：PID 结构体指针
  * @param    kp     ：比例系数
  * @param    ki     ：积分系数
  * @param    kd     ：微分系数
  * @retval   无
  */
void PID_SetParams(PID_TypeDef *pid, float kp, float ki, float kd)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

/**
  * @brief    执行一次位置式 PID 计算
  * @param    pid      ：PID 结构体指针
  * @param    measured ：当前测量值（反馈值）
  * @retval   PID 输出值（已限幅到 [out_min, out_max]）
  * @note     公式：output = Kp·e + Ki·∫e + Kd·(e - e_last)
  */
float PID_Compute(PID_TypeDef *pid, float measured)
{
    /* 更新误差历史 */
    pid->prev_error = pid->last_error;
    pid->last_error = pid->error;
    pid->error      = pid->target - measured;

    /* 积分累积 + 抗饱和限幅 */
    pid->integral += pid->error;
    pid->integral  = clamp(pid->integral, pid->integral_min, pid->integral_max);

    /* 位置式 PID 三项求和 */
    float p_term = pid->kp * pid->error;
    float i_term = pid->ki * pid->integral;
    float d_term = pid->kd * (pid->error - pid->last_error);

    pid->output = p_term + i_term + d_term;

    /* 输出限幅 */
    pid->output = clamp(pid->output, pid->out_min, pid->out_max);

    return pid->output;
}

/**
  * @brief    复位 PID 控制器（清零误差/积分/输出，保留参数不变）
  * @param    pid    ：PID 结构体指针
  * @retval   无
  */
void PID_Reset(PID_TypeDef *pid)
{
    pid->error      = 0.0f;
    pid->last_error = 0.0f;
    pid->prev_error = 0.0f;
    pid->integral   = 0.0f;
    pid->output     = 0.0f;
}
