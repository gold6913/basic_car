/**
 * @file    control.c
 * @brief   小车应用层控制模块实现
 * @note    封装定时器启停、角度旋转、速度/模式切换，供主循环和 target_start 调用
 */

#include "control.h"
#include "ti_msp_dl_config.h"
#include "encoder.h"
#include "pid.h"

/* ==================================================================================
 *  外部引用
 * ==================================================================================
 */

extern speed_mode_t move_mode;       /**< 当前运动模式（start.c） */
extern float_t measure_yaw;          /**< 偏航角测量值（start.c） */
extern PID_TypeDef pid_yaw;          /**< 偏航角 PID 控制器（start.c） */
extern PID_TypeDef pid_speed_L;      /**< 左轮速度 PID（start.c） */
extern PID_TypeDef pid_speed_R;      /**< 右轮速度 PID（start.c） */
extern PID_TypeDef pid_gray;         /**< 灰度 PID 控制器（start.c） */
extern uint8_t move;                 /**< 运动使能标志（start.c） */
extern char st1;                     /**< 阶段状态机（start.c） */

/* ==================================================================================
 *  外部速度变量（定义于 start.c）
 * ==================================================================================
 */

extern float gray_base_speed; /**< 灰度循迹基准速度（start.c） */
extern float yaw_base_speed;  /**< 偏航角控制基准速度（start.c） */

/* ==================================================================================
 *  定时器控制
 * ==================================================================================
 */

/**
 * @brief  启动 10ms 定时器（编码器速度环 + 转向环）
 */
void control_timer_start(void)
{
    NVIC_ClearPendingIRQ(encoder_INST_INT_IRQN);
    NVIC_EnableIRQ(encoder_INST_INT_IRQN);
    DL_TimerG_startCounter(encoder_INST);
}

/**
 * @brief  停止 10ms 定时器
 */
void control_timer_stop(void)
{
    NVIC_DisableIRQ(encoder_INST_INT_IRQN);
    DL_TimerG_stopCounter(encoder_INST);
}

/* ==================================================================================
 *  运动控制
 * ==================================================================================
 */

/**
 * @brief  旋转到绝对角度
 */
void control_rotate_to(float target_angle)
{
    move_mode = YAW_MODE;
    PID_SetTarget(&pid_yaw, target_angle);
}

/**
 * @brief  相对旋转
 */
void control_rotate_rel(float delta_angle)
{
    control_rotate_to(measure_yaw + delta_angle);
}

/**
 * @brief  设置循迹基准速度
 */
void control_set_base_speed(float speed)
{
    gray_base_speed = speed;
    PID_SetTarget(&pid_speed_L, speed);
    PID_SetTarget(&pid_speed_R, speed);
}

/**
 * @brief  设置偏航角基准速度
 */
void control_set_yaw_speed(float speed)
{
    yaw_base_speed = speed;
}

/* ==================================================================================
 *  模式切换
 * ==================================================================================
 */

/**
 * @brief  切换运动模式
 */
void control_set_mode(speed_mode_t mode)
{
    move_mode = mode;
}

/**
 * @brief  紧急停止
 */
void control_estop(void)
{
    move = 0;
    PID_SetTarget(&pid_speed_L, 0);
    PID_SetTarget(&pid_speed_R, 0);
}

/* ==================================================================================
 *  阶段控制
 * ==================================================================================
 */

/**
 * @brief  切换到下一阶段
 */
void control_next_stage(void)
{
    switch (st1)
    {
    case 'a': st1 = 'b'; break;
    case 'b': st1 = 'c'; break;
    case 'c': st1 = 'd'; break;
    default:  st1 = 'a'; break;
    }
}

/**
 * @brief  重置阶段到初始状态
 */
void control_reset_stage(void)
{
    st1 = 'a';
}
