#ifndef __START_H__
#define __START_H__

#include "ti_msp_dl_config.h"
#include "jy61p.h"
#include "pid.h"

/**
  * @brief  运动模式枚举
  *         SPEED_MODE : 纯速度模式，只跑速度环
  *         GRAY_MODE  : 灰度循迹模式，灰度环 + 速度环
  *         YAW_MODE   : 偏航角控制模式，角度环 + 速度环
  */
typedef enum {
    SPEED_MODE = 0,   /**< 纯速度模式 */
    GRAY_MODE,        /**< 灰度循迹模式 */
    YAW_MODE,          /**< 偏航角控制模式 */
    STOP_MODE
} speed_mode_t;

struct CAR_DATA
{
    uint16_t speed_right;
    uint16_t speed_left;
};

/*========================== 全局变量 (extern) ==========================*/

extern speed_mode_t move_mode;        /* 当前运动模式 */

extern const jy61p_data_t *imu;       /* IMU 陀螺仪数据指针 */
extern unsigned short gray[8];        /* 灰度传感器 8 通道二值化值(0/1) */
extern unsigned short black[8];       /* 灰度校准基准值 */
extern unsigned short gray_analog[8]; /* 灰度传感器 8 通道原始 ADC 值 */
extern PID_TypeDef pid_gray;          /* 灰度巡线 PID 控制器 */
extern PID_TypeDef pid_speed_L;       /* 左轮速度 PID 控制器 */
extern PID_TypeDef pid_speed_R;       /* 右轮速度 PID 控制器 */
extern PID_TypeDef pid_yaw;           /* 偏航角 PID 控制器 */
extern float_t measure_yaw;           /* 偏航角测量值 */
extern int measure_gray;              /* 灰度偏差测量值 */
extern uint8_t move;                  /* 运动使能标志: 1=运动, 0=停止 */
extern char st1;                      /* 阶段状态机: a=循迹 b=等待转弯 c=转弯中 */
extern float gray_base_speed;         /* 灰度循迹基准速度（脉冲/周期） */
extern float yaw_base_speed;          /* 偏航角控制基准速度（脉冲/周期） */
extern float gray_correction;         /* 灰度 PID 修正量 */
extern float gray_target_L;           /* 灰度 PID 后左轮速度目标 */
extern float gray_target_R;           /* 灰度 PID 后右轮速度目标 */

/*========================== 函数声明 ==========================*/

void init(void);
void loading_show(void);
void Collect_Data(void);
void motion_control(float_t speed_right, float_t speed_left, uint8_t a);
void judge(void);
void target_start(void);

#endif
