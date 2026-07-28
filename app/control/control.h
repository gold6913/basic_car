/**
 * @file    control.h
 * @brief   小车应用层控制模块 — 封装定时器启停、角度旋转、速度/模式切换等操作
 */

#ifndef __CONTROL_H__
#define __CONTROL_H__

#include <stdint.h>
#include "start.h"

#ifdef __cplusplus
extern "C" {
#endif

/*==================================================================
 *  定时器控制
 *==================================================================*/

/** @brief  启动 10ms 定时器（编码器速度环 + 转向环） */
void control_timer_start(void);

/** @brief  停止 10ms 定时器（禁用 NVIC + 停计数器） */
void control_timer_stop(void);

/*==================================================================
 *  运动控制
 *==================================================================*/

/**
 * @brief  旋转到绝对角度（偏航角控制）
 * @param  target_angle ：目标偏航角度（°）
 * @note   内部设置 move_mode = YAW_MODE，PID 目标 = target_angle
 */
void control_rotate_to(float target_angle);

/**
 * @brief  相对旋转
 * @param  delta_angle ：相对角度增量（°），正=右转，负=左转
 * @note   基于当前 measure_yaw + delta_angle 计算绝对目标
 */
void control_rotate_rel(float delta_angle);

/**
 * @brief  设置循迹基准速度
 * @param  speed ：基准速度（脉冲/周期）
 */
void control_set_base_speed(float speed);

/**
 * @brief  设置偏航角基准速度
 * @param  speed ：基准速度（脉冲/周期）
 */
void control_set_yaw_speed(float speed);

/*==================================================================
 *  模式切换
 *==================================================================*/

/**
 * @brief  切换运动模式
 * @param  mode ：目标模式（SPEED_MODE / GRAY_MODE / YAW_MODE）
 */
void control_set_mode(speed_mode_t mode);

/**
 * @brief  紧急停止（模式不变，速度目标清零）
 */
void control_estop(void);

/*==================================================================
 *  阶段控制（多阶段任务用）
 *==================================================================*/

/** @brief  切换到下一阶段（st1 状态机步进） */
void control_next_stage(void);

/** @brief  重置阶段到初始状态（st1 = 'a'） */
void control_reset_stage(void);

#ifdef __cplusplus
}
#endif

#endif /* __CONTROL_H__ */
