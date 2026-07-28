/**
 * @file    jy61p.h
 * @brief   JY61P六轴IMU传感器驱动（UART中断接收 + 命令控制）
 *
 * @details 驱动功能分为两大部分：
 *
 * 【数据接收】通过 UART RX 中断接收 JY61P 模块数据，
 *   ISR 内状态机逐字节解析，校验通过后更新 jy61p_data_t 结构体。
 *   主循环通过 jy61p_get_data() 只读访问最新数据，
 *   通过 updated 标志判断是否有新数据需要处理。
 *
 * 【命令控制】支持向 JY61P 发送校准/归零命令：
 *   - 加速度/陀螺仪/磁场校准（3字节短命令）
 *   - XY/Z轴角度归零（5字节寄存器写入协议，含解锁+保存）
 *
 * @note    JY61P 默认波特率 9600，需在 SysConfig 中将对应 UART 配置为 9600。
 *          初始化时必须调用 jy61p_init() 以启用 NVIC 中断。
 *          支持更换串口号，修改 JY61P_UART_PORT 宏即可。
 *
 * 典型使用流程：
 *   SYSCFG_DL_init();
 *   jy61p_init();
 *   while (1) {
 *       const jy61p_data_t *imu = jy61p_get_data();
 *       if (imu->angle_updated) {
 *           // 已换算为度(°)，直接使用 imu->angle_roll
 *           jy61p_clear_flags();
 *       }
 *   }
 */

#ifndef __JY61P_H__
#define __JY61P_H__

#include <stdint.h>
#include "uart.h"

/** JY61P连接的串口号，修改此宏即可切换串口 */
#define JY61P_UART_PORT  UART_PORT_3

/**
 * @brief  IMU 模块型号（自动检测结果）
 */
typedef enum {
    JY61P_MODEL_NONE  = 0,  /**< 未检测到模块（RX 悬空噪声） */
    JY61P_MODEL_JY60  = 1,  /**< JY60：仅命令控制，无数据回传 */
    JY61P_MODEL_JY61P = 2,  /**< JY61P：数据帧 + 命令控制 */
} jy61p_model_t;

/**
 * @brief JY61P 解析后的数据结构
 *
 * 字段值已在驱动内部完成换算，读取后直接使用即可：
 *   加速度(g)    —— 量程 ±16g，单位：g
 *   角速度(°/s)  —— 量程 ±2000°/s，单位：°/s
 *   欧拉角(°)    —— 范围 ±180°，单位：°
 *
 * @note 内部换算公式：
 *   float_val = raw_int16 / 32768.0f × 满量程
 *   所有原始数据为小端序有符号 16 位整数，正负方向取决于模块安装方向。
 */
typedef struct {
    /* 加速度 (单位: g, 量程 ±16g, 已换算) */
    float acc_x;    /**< X轴加速度 (g) */
    float acc_y;    /**< Y轴加速度 (g) */
    float acc_z;    /**< Z轴加速度 (g) */

    /* 角速度 (单位: °/s, 量程 ±2000°/s, 已换算) */
    float gyro_x;   /**< X轴角速度 (°/s) */
    float gyro_y;   /**< Y轴角速度 (°/s) */
    float gyro_z;   /**< Z轴角速度 (°/s) */

    /* 欧拉角 (单位: °, 范围 ±180°, 已换算) */
    float angle_roll;  /**< 横滚角 (°) */
    float angle_pitch; /**< 俯仰角 (°) */
    float angle_yaw;   /**< 偏航角 (°) */

    /* RTC时间 (0x50帧输出, JY61P内置实时时钟) */
    uint8_t hour;   /**< 时 (0-23) */
    uint8_t minute; /**< 分 (0-59) */
    uint8_t second; /**< 秒 (0-59) */

    /* 数据更新标志 */
    uint8_t acc_updated;   /**< 加速度数据已更新 */
    uint8_t gyro_updated;  /**< 角速度数据已更新 */
    uint8_t angle_updated; /**< 角度数据已更新 */
    uint8_t time_updated;  /**< 时间数据已更新 */
} jy61p_data_t;

/**
 * @brief  获取JY61P数据指针（只读访问）
 * @retval 指向内部数据结构的const指针
 */
const jy61p_data_t *jy61p_get_data(void);

/**
 * @brief  清除所有数据更新标志
 * @note   在读取完数据后调用，用于判断下次是否有新数据
 */
void jy61p_clear_flags(void);

/**
 * @brief  JY61P初始化（启用NVIC中断）
 * @note   必须在 SYSCFG_DL_init() 之后调用，否则中断无法触发。
 */
void jy61p_init(void);

/**
 * @brief  自动检测 IMU 模块型号
 * @retval JY61P_MODEL_JY61P — 收到有效数据帧
 *         JY61P_MODEL_JY60  — RX 无中断（引脚被驱动），但无数据帧
 *         JY61P_MODEL_NONE  — RX 有噪声中断但无有效帧（引脚悬空）
 * @note   在 jy61p_init() 之后延时约 200ms 再调用
 */
jy61p_model_t jy61p_detect(void);

/**
 * @brief  JY61P轮询接口（已废弃，保留兼容）
 * @note   中断模式下此函数为空实现，数据由中断服务函数自动更新。
 */
void jy61p_poll(void);

/* ======================== 校准/控制命令 ======================== */
/*
 * 所有命令均需要 解锁→写入→保存 三步流程，
 * 内部自动处理解锁和保存，调用者只需调用对应函数。
 * 注意：Z轴归零仅在六轴算法模式下有效，九轴模式下为绝对角度无法归零。
 */

/**
 * @brief  加速度校准
 * @note   将模块水平放置后调用，校准过程中保持静止。
 *         校准完成后模块会自动重启，约需 2~3 秒。
 */
void jy61p_calibrate_acc(void);

/**
 * @brief  陀螺仪归零（零偏校准）
 * @note   将模块静止放置后调用，校准过程中绝对不可移动。
 *         校准完成后模块会自动重启，约需 2~3 秒。
 */
void jy61p_calibrate_gyro(void);

/**
 * @brief  磁场校准
 * @note   调用后需将模块在空间中旋转 360°，约 10 秒内完成。
 *         JY61P 若不带磁力计则此命令无效。
 */
void jy61p_calibrate_mag(void);

/**
 * @brief  XY轴角度归零（将当前 Roll/Pitch 设为 0）
 * @note   以当前姿态为参考点，XY 轴角度输出变为 0。
 */
void jy61p_reset_angle_xy(void);

/**
 * @brief  Z轴角度归零（将当前 Yaw 设为 0）
 * @note   仅在六轴算法模式下有效，九轴模式下 Yaw 为绝对角度无法归零。
 */
void jy61p_reset_angle_z(void);

/**
 * @brief  全轴角度归零（XY + Z 同时归零）
 * @note   等同于依次调用 reset_angle_xy() 和 reset_angle_z()。
 */
void jy61p_reset_angle(void);

#endif
