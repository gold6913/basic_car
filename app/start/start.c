/**
 * @file    start.c
 * @brief   系统初始化与运动控制核心模块
 * @note    负责外设初始化、数据采集、双轮差速运动控制及定时器中断服务
 */

#include "start.h"
#include "No_Mcu_Ganv_Grayscale_Sensor_Config.h"
#include "borad.h"
#include "delay.h"
#include "encoder.h"
#include "jy61p.h"
#include "menu.h"
#include "oled.h"
#include "pid.h"
#include "tb6612.h"
#include "zdt_motor.h"

/* ==================================================================================
 *  全局变量
 * ==================================================================================
 */

extern uint8_t menu_cursor; /* 菜单光标/目标选择（由 menu 模块维护） */
extern uint8_t menu_start;

const jy61p_data_t *imu; /* IMU 姿态数据指针 */
unsigned short gray[8];  /* 灰度传感器 8 通道原始值 */
unsigned short black[8] = {2888, 2888, 2888, 2888,
                           2000, 2000, 2000, 2000}; /* 灰度校准基准值 */

PID_TypeDef pid_gray;    /* 灰度循迹 PID 控制器（外环） */
PID_TypeDef pid_speed_L; /* 左轮速度 PID 控制器（内环） */
PID_TypeDef pid_speed_R; /* 右轮速度 PID 控制器（内环） */
PID_TypeDef pid_yaw;     /* 偏航角 PID 控制器 */
PID_TypeDef pid_yaw33; /* 偏航角 PID 控制器（33° 目标） */

int measure_gray;                    /* 灰度偏差测量值（PID 输入） */
float_t measure_yaw;                 /* 偏航角测量值（PID 输入） */
speed_mode_t move_mode = GRAY_MODE; /* 当前运动模式 */
uint8_t move = 1;                    /* 运动使能标志：1=运动 0=停止 */
char st1 = 'a';                      /* 阶段状态机：a=循迹段 b=等待转弯 c=转弯中 */
uint8_t d = 0;
/* ==================================================================================
 *  功能函数
 * ==================================================================================
 */

/**
 * @brief    目标启动调度函数
 * @note     根据 menu_cursor 选择运行模式：case 0 循迹 / case 1 多阶段 / case 2 灰度停车
 * @retval   无
 */
void target_start(void)
{
    switch (menu_cursor)
    {
    case 0:

    case 1:

    case 2:

    case 3:
    default:
        break;
    }
}

/**
 * @brief    系统初始化
 * @note     依次完成：系统外设、OLED、IMU、定时器配置、PID 初始化
 *           - 定时器 1 (TimerA)：灰度循迹采样
 *           - 定时器 G (gray_INST)：灰度转向环 + 速度环
 *           - 定时器 3：偏航角 33° PID 控制
 * @retval   无
 */
void init(void)
{
    SYSCFG_DL_init();       /* 系统外设初始化（由 syscfg 自动生成） */
    OLED_Init();            /* OLED 屏幕初始化 */
    jy61p_init();           /* JY61P 姿态传感器串口初始化 */
    imu = jy61p_get_data(); /* 获取 IMU 数据访问指针 */

    /* ---- 定时器 1 配置（灰度循迹采样） ---- */
    NVIC_ClearPendingIRQ(OLED_refresh_INST_INT_IRQN); /* 清除定时器 1 中断标志 */
    NVIC_EnableIRQ(OLED_refresh_INST_INT_IRQN);       /* 使能 NVIC 定时器 1 中断 */
    DL_TimerA_startCounter(OLED_refresh_INST);        /* 启动定时器 1 计数器 */

    /* ---- 编码器 GPIO 中断 + 10ms 定时器 ---- */
    encoder_init();                              /* 编码器 GPIO 脉冲计数 */


    /* ---- 定时器 3 配置（偏航角 33° PID） ---- */

    OLED_ShowString(0, 24, (u8 *)"wait for init...", 16, 1); /* OLED 显示等待提示 */
    OLED_Refresh();

    /* 自动检测 IMU 模块型号（延时 200ms 等待 JY61P 上电发送数据） */
    delay_ms(200);
    switch (jy61p_detect()) {
    case JY61P_MODEL_JY61P:
        jy61p_reset_angle();            /* JY61P 在线 → 角度归零 */
        break;
    case JY61P_MODEL_JY60:
        jy61p_reset_angle();            /* JY60 在线 → 角度归零（仅发命令） */
        break;
    case JY61P_MODEL_NONE:
    default:
        NVIC_DisableIRQ(UART_3_INST_INT_IRQN);  /* 未检测到 → 关中断防悬空噪声 */
        break;
    }
    OLED_Clear();                   /* 清除 OLED 屏幕 */

    /* ---- 速度环 PID 初始化（内环，左右轮独立） ---- */
    PID_Init(&pid_speed_L, 4.0f, 2.0f, 0.0f, -1000.0f, 1000.0f);
    PID_SetTarget(&pid_speed_L, 0);
    PID_Init(&pid_speed_R, 4.0f, 2.0f, 0.0f, -1000.0f, 1000.0f);
    PID_SetTarget(&pid_speed_R, 0);
    PID_Init(&pid_gray, 0.3f, 0.0000f,5.0f, -40.0f, 40.0f);
    PID_SetTarget(&pid_gray, 0);
    PID_Init(&pid_yaw, -0.3f, 0.00001f, -1.0f, -40.0f, 40.0f);
    PID_SetTarget(&pid_yaw, 90);
}

/**
  * @brief    模式切换加载画面
  * @note     根据 menu_cursor 选择不同运行模式，加载对应参数后跳转运行
  * @param    无
  * @retval   无
  */
void loading_show(void)
{
    OLED_Clear();
    /* 根据菜单选择显示对应的模式名称 */
    switch (menu_cursor)
    {
    case 0:
        OLED_ShowString(0, 24, (u8 *)"target1", 16, 1);
        move_mode = GRAY_MODE;
        break;
    case 1:
        OLED_ShowString(0, 24, (u8 *)"target2", 16, 1);
        move_mode = GRAY_MODE;
        break;
    case 2:
        OLED_ShowString(0, 24, (u8 *)"target3", 16, 1);
        move_mode = GRAY_MODE;
        break;
    case 3:
        OLED_ShowString(0, 24, (u8 *)"target4", 16, 1);
        move_mode = YAW_MODE;
        break;
    default:
        OLED_ShowString(0, 24, (u8 *)"Loading...", 16, 1);
        move_mode = GRAY_MODE;
        break;
    }
    OLED_Refresh();
    delay_ms(2000);            /* 等待 2 秒 */
    NVIC_ClearPendingIRQ(encoder_INST_INT_IRQN); /* 清除定时器中断标志 */
    NVIC_EnableIRQ(encoder_INST_INT_IRQN);       /* 使能 NVIC 定时器中断 */
    DL_TimerG_startCounter(encoder_INST);        /* 启动 10ms 定时器 */
    menu_start = MODE_RUN;            /* 跳转到运行模式 */
}

/**
 * @brief    采集传感器数据
 * @note     将 8 通道灰度模拟值写入 gray 数组，以 black 数组为校准基准；
 *           同时读取 IMU 偏航角并映射到 [-180, 180)
 * @retval   无
 */
void Collect_Data(void)
{
    measure_gray = Get_Analog_value(gray, black);
    measure_yaw = imu->angle_yaw;
    /*双180模式。用于正反跑*/
    // if (measure_yaw >= 90 && measure_yaw <= 180)
    // {
    //     measure_yaw = measure_yaw - 180;
    // }
    // else if (measure_yaw < -90 && measure_yaw >= -180)
    // {    
    //     measure_yaw = measure_yaw + 180;
    // }
    /*360度模式，用于直角转弯*/
        if (measure_yaw >= 90 && measure_yaw <= 180)
    {
        measure_yaw = -measure_yaw + 180;
    }
        else if (measure_yaw < -90 && measure_yaw >= -180)
    {
        measure_yaw = -measure_yaw - 180;
    }
}

/**
 * @brief    双轮差速运动控制
 * @param    speed_right ：右轮速度
 *                         > 0 : 前进 (CW)
 *                         < 0 : 后退 (CCW)
 * @param    speed_left  ：左轮速度
 *                         > 0 : 前进 (CCW)
 *                         < 0 : 后退 (CW)
 * @param    a           ：运动使能
 *                         = 1 : 按速度运行
 *                         = 0 : 立即停止
 * @note     左右电机对向安装，相同正负值时实际转向相反，
 *           即 speed_right=500, speed_left=500 时小车直行前进
 * @retval   无
 */
void motion_control(float_t speed_right, float_t speed_left, uint8_t a)
{
    motor_set(speed_left, speed_right, a);
}

/**********************************************************
*** 速度环（内环）—— 编码器反馈 + PWM 输出
**********************************************************/

/**
  * @brief    编码器读取 + 速度控制循环
  * @note     1. 读取并清零编码器脉冲 → 2. PID 计算 → 3. PWM 输出
  * @param    pid_L ：左轮 PID 控制器
  * @param    pid_R ：右轮 PID 控制器
  * @retval   无
  */
static void speed_control_loop(PID_TypeDef *pid_L, PID_TypeDef *pid_R)
{
    /*==================== 1. 读取并清零编码器脉冲 ====================*/
    int32_t cur_left  = encoder_left_count;
    int32_t cur_right = encoder_right_count;
    encoder_left_count  = 0;
    encoder_right_count = 0;

    encoder_left_speed  = cur_left;   /* 记录当前速度（脉冲/周期） */
    encoder_right_speed = cur_right;

    /*==================== 2. PID 计算 ====================*/
    float out_L = PID_Compute(pid_L, (float)cur_left);
    float out_R = PID_Compute(pid_R, (float)cur_right);

    /*==================== 3. 输出 PWM ====================*/
    motor_set(out_R, out_L, 1);
}
/** @brief 灰度循迹基准速度（左右轮共同的基础速度，脉冲/周期） */
float gray_base_speed = 10.0f;

/**
 * @brief    灰度循迹转向控制循环
 * @note     读取灰度偏差 → PID 计算转向修正量 → 更新左右轮速度目标
 *           修正量 > 0：车偏右，左轮加速、右轮减速（向左修正）
 *           修正量 < 0：车偏左，左轮减速、右轮加速（向右修正）
 * @param    pid ：灰度 PID 控制器
 * @retval   无
 */
static void gray_steer_loop(PID_TypeDef *pid)
{
    /*==================== 1. 读取灰度偏差 ====================*/
    /* measure_gray 由主函数中 Collect_Data 提前更新 */

    /*==================== 2. PID 计算转向修正量 ====================*/
    float correction = PID_Compute(pid, (float)measure_gray);

    /*==================== 3. 差速分配：更新速度环目标 ====================*/
    PID_SetTarget(&pid_speed_L, gray_base_speed - correction);
    PID_SetTarget(&pid_speed_R, gray_base_speed + correction);
}

/**********************************************************
*** 转向环（外环）—— 偏航角控制
**********************************************************/

/** @brief 偏航角控制基准速度（脉冲/周期） */
float yaw_base_speed = 10.0f;

/**
 * @brief    偏航角转向控制循环
 * @note     设置目标角度 → 读取偏航角 → PID 计算转向修正量 → 更新左右轮速度目标
 *           修正量 > 0：当前角度大于目标（偏右），左轮加速、右轮减速
 *           修正量 < 0：当前角度小于目标（偏左），左轮减速、右轮加速
 * @param    pid          ：偏航角 PID 控制器
 * @param    target_angle ：目标偏航角度
 * @retval   无
 */
static void yaw_steer_loop(PID_TypeDef *pid, float target_angle)
{
    /*==================== 1. 更新目标角度 ====================*/
    PID_SetTarget(pid, target_angle);

    /*==================== 2. 读取偏航角 ====================*/
    /* measure_yaw 由主函数中 Collect_Data 提前更新 */

    /*==================== 3. PID 计算转向修正量 ====================*/
    float correction = PID_Compute(pid, measure_yaw);

    /*==================== 4. 差速分配：更新速度环目标 ====================*/
    PID_SetTarget(&pid_speed_L, yaw_base_speed + correction);
    PID_SetTarget(&pid_speed_R, yaw_base_speed - correction);
}

/**********************************************************
*** 定时器中断服务函数
**********************************************************/

/**
 * @brief    编码器速度环中断服务函数（TIMG0，10ms 周期）
 * @note     通过 speed_control_loop + pid_compute_cb 实现速度闭环
 * @retval   无
 */
void encoder_INST_IRQHandler(void)
{
    switch (move_mode)
    {
    /*========================== 纯速度模式 ==========================*/
    case SPEED_MODE:
        speed_control_loop(&pid_speed_L, &pid_speed_R);
        break;

    /*========================== 灰度循迹模式 ==========================*/
    case GRAY_MODE:
        gray_steer_loop(&pid_gray);
        speed_control_loop(&pid_speed_L, &pid_speed_R);
        break;

    /*========================== 偏航角控制模式 ==========================*/
    case YAW_MODE:
        yaw_steer_loop(&pid_yaw, 0.0f);
        speed_control_loop(&pid_speed_L, &pid_speed_R);
        break;

    /*========================== 默认：纯速度 ==========================*/
    default:
        speed_control_loop(&pid_speed_L, &pid_speed_R);
        break;
    }
}