/**
 * @file    pid.h
 * @brief   PID 控制器结构体定义与接口声明
 */

#ifndef __PID_H__
#define __PID_H__

#include <stdint.h>

/**
 * @brief  单路 PID 控制器实例结构体
 * @note   每个需要独立 PID 控制的通道各定义一个实例，
 *         所有状态与参数均封装在结构体内部，与系统完全解耦。
 */
typedef struct {
    /* ---- 控制参数 ---- */
    float kp;               /**< 比例系数 */
    float ki;               /**< 积分系数 */
    float kd;               /**< 微分系数 */

    /* ---- 目标与误差 ---- */
    float target;           /**< 目标值（设定值） */
    float error;            /**< 当前误差 */
    float last_error;       /**< 上一次误差（用于微分项） */
    float prev_error;       /**< 上上次误差（可选，用于二阶微分） */

    /* ---- 积分累积 ---- */
    float integral;         /**< 积分项累积值 */

    /* ---- 输出限幅 ---- */
    float out_max;          /**< 输出上限 */
    float out_min;          /**< 输出下限 */

    /* ---- 积分限幅（抗饱和） ---- */
    float integral_max;     /**< 积分上限 */
    float integral_min;     /**< 积分下限 */

    /* ---- 输出结果 ---- */
    float output;           /**< 最近一次 PID 计算输出 */
} PID_TypeDef;

/**
 * @brief    初始化 PID 控制器
 * @param    pid       PID 实例指针
 * @param    kp        比例系数
 * @param    ki        积分系数
 * @param    kd        微分系数
 * @param    out_min   输出下限
 * @param    out_max   输出上限
 */
void PID_Init(PID_TypeDef *pid, float kp, float ki, float kd,
              float out_min, float out_max);

/**
 * @brief    设置 PID 目标值
 * @param    pid      PID 实例指针
 * @param    target   目标值
 */
void PID_SetTarget(PID_TypeDef *pid, float target);

/**
 * @brief    在线修改 PID 参数
 * @param    pid  PID 实例指针
 * @param    kp   比例系数
 * @param    ki   积分系数
 * @param    kd   微分系数
 */
void PID_SetParams(PID_TypeDef *pid, float kp, float ki, float kd);

/**
 * @brief    计算一次 PID 输出
 * @param    pid       PID 实例指针
 * @param    measured  当前测量值（反馈值）
 * @retval   PID 计算输出（已限幅）
 */
float PID_Compute(PID_TypeDef *pid, float measured);

/**
 * @brief    清除 PID 内部状态（积分、误差历史），参数不变
 * @param    pid  PID 实例指针
 */
void PID_Reset(PID_TypeDef *pid);

#endif
