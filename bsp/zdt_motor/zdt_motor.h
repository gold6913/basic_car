#ifndef __ZDT_MOTOR_H__
#define __ZDT_MOTOR_H__

#include "ti_msp_dl_config.h"
#include "uart.h"

/*==========================================================
 * 宏定义 & 全局变量
 *==========================================================*/
#define uart        UART_PORT_2
#define MMCL_LEN    512

extern uint16_t MMCL_count;                 /* 多电机命令当前字节计数 */
extern uint16_t MMCL_cmd[MMCL_LEN];         /* 多电机命令缓冲区 */

/*==========================================================
 * 系统参数类型枚举
 *==========================================================*/
typedef enum {
    S_VBUS  = 5,    /* 总线电压 */
    S_CBUS  = 6,    /* 总线电流 */
    S_CPHA  = 7,    /* 相电流 */
    S_ENCO  = 8,    /* 编码器原始值 */
    S_CLKC  = 9,    /* 实时脉冲数 */
    S_ENCL  = 10,   /* 线性化校准编码器值 */
    S_CLKI  = 11,   /* 输入脉冲数 */
    S_TPOS  = 12,   /* 电机目标位置 */
    S_SPOS  = 13,   /* 电机实时设定目标位置 */
    S_VEL   = 14,   /* 电机实时转速 */
    S_CPOS  = 15,   /* 电机实时位置 */
    S_PERR  = 16,   /* 电机位置误差 */
    S_VBAT  = 17,   /* 多圈编码器电池电压（Y42） */
    S_TEMP  = 18,   /* 电机实时温度（Y42） */
    S_FLAG  = 19,   /* 电机状态标志位 */
    S_OFLAG = 20,   /* 回零状态标志位 */
    S_OAF   = 21,   /* 电机状态 + 回零状态标志位（Y42） */
    S_PIN   = 22,   /* 引脚状态（Y42） */
} SysParams_t;


/*==========================================================
 * 基础工具函数
 *==========================================================*/

/**
 * @brief    向电机串口发送命令缓冲区
 * @param    buf  命令数据指针
 * @param    len  命令数据长度（字节数）
 */
void usart_SendCmd(const uint8_t *buf, size_t len);


/*##########################################################
 *#
 *#  第一部分：单电机直接发送命令
 *#  调用后立即通过串口发送对应命令帧
 *#
 *##########################################################*/

/*----------------------------------------------------------
 * 1.1 触发动作命令
 *----------------------------------------------------------*/

/** @brief 触发编码器校准 */
void Emm_V5_Trig_Encoder_Cal(uint8_t addr);

/** @brief 重启电机（Y42） */
void Emm_V5_Reset_Motor(uint8_t addr);

/** @brief 将当前位置清零 */
void Emm_V5_Reset_CurPos_To_Zero(uint8_t addr);

/** @brief 解除堵转保护 */
void Emm_V5_Reset_Clog_Pro(uint8_t addr);

/** @brief 恢复出厂设置 */
void Emm_V5_Restore_Motor(uint8_t addr);

/*----------------------------------------------------------
 * 1.2 运动控制命令
 *----------------------------------------------------------*/

/**
 * @brief    使能信号控制
 * @param    addr  ：电机地址
 * @param    state ：使能状态，true为使能电机，false为关闭电机
 * @param    snF   ：多机同步标志，false为不启用，true为启用
 */
void Emm_V5_En_Control(uint8_t addr, bool state, bool snF);

/**
 * @brief    速度模式
 * @param    addr ：电机地址
 * @param    dir  ：方向，0为CW，其余值为CCW
 * @param    vel  ：速度，范围0 - 5000RPM
 * @param    acc  ：加速度，范围0 - 255，注意：0是直接启动
 * @param    snF  ：多机同步标志，false为不启用，true为启用
 */
void Emm_V5_Vel_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF);

/**
 * @brief    位置模式
 * @param    addr ：电机地址
 * @param    dir  ：方向，0为CW，其余值为CCW
 * @param    vel  ：速度(RPM)，范围0 - 5000RPM
 * @param    acc  ：加速度，范围0 - 255，注意：0是直接启动
 * @param    clk  ：脉冲数，范围0 - (2^32 - 1)个
 * @param    raF  ：相位置/绝对标志，false为相对运动，true为绝对值运动
 * @param    snF  ：多机同步标志，false为不启用，true为启用
 */
void Emm_V5_Pos_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, bool raF, bool snF);

/**
 * @brief    立即停止
 * @param    addr ：电机地址
 * @param    snF  ：多机同步标志，false为不启用，true为启用
 */
void Emm_V5_Stop_Now(uint8_t addr, bool snF);

/** @brief 多机同步运行 */
void Emm_V5_Synchronous_motion(uint8_t addr);

/**
 * @brief    多电机命令（Y42）
 * @param    addr ：电机地址
 * @note     先调用 MMCL_ 系列函数装载子命令，再调用此函数一次性发送
 */
void Emm_V5_Multi_Motor_Cmd(uint8_t addr);

/*----------------------------------------------------------
 * 1.3 原点回零命令
 *----------------------------------------------------------*/

/**
 * @brief    设置单圈回零的零点位置
 * @param    addr ：电机地址
 * @param    svF  ：是否存储标志，false为不存储，true为存储
 */
void Emm_V5_Origin_Set_O(uint8_t addr, bool svF);

/**
 * @brief    触发回零
 * @param    addr   ：电机地址
 * @param    o_mode ：回零模式，0=单圈就近，1=单圈方向，2=多圈碰撞，3=多圈限位开关
 * @param    snF    ：多机同步标志
 */
void Emm_V5_Origin_Trigger_Return(uint8_t addr, uint8_t o_mode, bool snF);

/** @brief 强制中断并退出回零 */
void Emm_V5_Origin_Interrupt(uint8_t addr);

/** @brief 读取回零参数 */
void Emm_V5_Origin_Read_Params(uint8_t addr);

/**
 * @brief    修改回零参数
 * @param    addr   ：电机地址
 * @param    svF    ：是否存储标志
 * @param    o_mode ：回零模式，0=单圈就近，1=单圈方向，2=多圈碰撞，3=多圈限位开关
 * @param    o_dir  ：回零方向，0为CW，其余值为CCW
 * @param    o_vel  ：回零速度(RPM)
 * @param    o_tm   ：回零超时时间(ms)
 * @param    sl_vel ：碰撞检测转速(RPM)
 * @param    sl_ma  ：碰撞检测电流(mA)
 * @param    sl_ms  ：碰撞检测时间(ms)
 * @param    potF   ：上电自动触发回零
 */
void Emm_V5_Origin_Modify_Params(uint8_t addr, bool svF, uint8_t o_mode, uint8_t o_dir, uint16_t o_vel, uint32_t o_tm, uint16_t sl_vel, uint16_t sl_ma, uint16_t sl_ms, bool potF);

/*----------------------------------------------------------
 * 1.4 读取系统参数命令
 *----------------------------------------------------------*/

/**
 * @brief    定时返回信息命令（Y42）
 * @param    addr    ：电机地址
 * @param    s       ：系统参数类型（SysParams_t）
 * @param    time_ms ：定时时间
 */
void Emm_V5_Auto_Return_Sys_Params_Timed(uint8_t addr, SysParams_t s, uint16_t time_ms);

/**
 * @brief    读取系统参数
 * @param    addr ：电机地址
 * @param    s    ：系统参数类型（SysParams_t）
 */
void Emm_V5_Read_Sys_Params(uint8_t addr, SysParams_t s);

/*----------------------------------------------------------
 * 1.5 读写驱动参数命令 — 基本配置
 *----------------------------------------------------------*/

/** @brief 修改电机ID地址（1-255，0为广播） */
void Emm_V5_Modify_Motor_ID(uint8_t addr, bool svF, uint8_t id);

/** @brief 修改细分值（默认16，0=256细分） */
void Emm_V5_Modify_MicroStep(uint8_t addr, bool svF, uint8_t mstep);

/** @brief 修改掉电标志 */
void Emm_V5_Modify_PDFlag(uint8_t addr, bool pdf);

/** @brief 读取选项参数状态（Y42） */
void Emm_V5_Read_Opt_Param_Sta(uint8_t addr);

/*----------------------------------------------------------
 * 1.5 读写驱动参数命令 — 电机 & 固件配置（Y42）
 *----------------------------------------------------------*/

/** @brief 修改电机类型，0=1.8度，1=0.9度 */
void Emm_V5_Modify_Motor_Type(uint8_t addr, bool svF, bool mottype);

/** @brief 修改固件类型，0=X固件，1=Emm固件 */
void Emm_V5_Modify_Firmware_Type(uint8_t addr, bool svF, bool fwtype);

/** @brief 修改控制模式，0=开环，1=闭环FOC */
void Emm_V5_Modify_Ctrl_Mode(uint8_t addr, bool svF, bool ctrl_mode);

/** @brief 修改运动正方向，0=CW，1=CCW */
void Emm_V5_Modify_Motor_Dir(uint8_t addr, bool svF, bool dir);

/** @brief 修改锁定按键功能，0=Disable，1=Enable */
void Emm_V5_Modify_Lock_Btn(uint8_t addr, bool svF, bool lock);

/** @brief 修改命令速度值是否缩小10倍，0=Disable，1=Enable */
void Emm_V5_Modify_S_Vel(uint8_t addr, bool svF, bool s_vel);

/*----------------------------------------------------------
 * 1.5 读写驱动参数命令 — 电流 & PID
 *----------------------------------------------------------*/

/** @brief 修改开环模式工作电流(mA) */
void Emm_V5_Modify_OM_mA(uint8_t addr, bool svF, uint16_t om_ma);

/** @brief 修改闭环模式最大电流(mA) */
void Emm_V5_Modify_FOC_mA(uint8_t addr, bool svF, uint16_t foc_mA);

/** @brief 读取PID参数 */
void Emm_V5_Read_PID_Params(uint8_t addr);

/** @brief 修改PID参数（kp/ki/kd） */
void Emm_V5_Modify_PID_Params(uint8_t addr, bool svF, uint32_t kp, uint32_t ki, uint32_t kd);

/** @brief 读取积分限幅/刚性系数 */
void Emm_V5_Read_Integral_Limit(uint8_t addr);

/** @brief 修改积分限幅/刚性系数（默认65535） */
void Emm_V5_Modify_Integral_Limit(uint8_t addr, bool svF, uint32_t il);

/*----------------------------------------------------------
 * 1.5 读写驱动参数命令 — 保护参数（Y42）
 *----------------------------------------------------------*/

/** @brief 读取过热过流保护检测阈值 */
void Emm_V5_Read_Otocp(uint8_t addr);

/**
 * @brief    修改过热过流保护检测阈值
 * @param    otp     ：过热保护阈值，默认100度
 * @param    ocp     ：过流保护阈值，默认6600mA
 * @param    time_ms ：检测时间，默认1000ms
 */
void Emm_V5_Modify_Otocp(uint8_t addr, bool svF, uint16_t otp, uint16_t ocp, uint16_t time_ms);

/** @brief 读取心跳保护功能时间 */
void Emm_V5_Read_Heart_Protect(uint8_t addr);

/** @brief 修改心跳保护功能时间(ms) */
void Emm_V5_Modify_Heart_Protect(uint8_t addr, bool svF, uint32_t hp);

/** @brief 读取位置到达窗口 */
void Emm_V5_Read_Pos_Window(uint8_t addr);

/** @brief 修改位置到达窗口（默认8，即0.8度） */
void Emm_V5_Modify_Pos_Window(uint8_t addr, bool svF, uint16_t prw);

/*----------------------------------------------------------
 * 1.5 读写驱动参数命令 — DMX512协议（Y42）
 *----------------------------------------------------------*/

/** @brief 读取DMX512协议参数 */
void Emm_V5_Read_DMX512_Params(uint8_t addr);

/**
 * @brief    修改DMX512协议参数
 * @param    tch      ：总通道数，默认192
 * @param    nch      ：每电机通道数，默认1
 * @param    mode     ：运动模式，0=相对位置，1=绝对坐标
 * @param    vel      ：单通道速度，默认1000RPM
 * @param    acc      ：加速度
 * @param    vel_step ：双通道速度步长，默认10
 * @param    pos_step ：双通道运动步长，默认100
 */
void Emm_V5_Modify_DMX512_Params(uint8_t addr, bool svF, uint16_t tch, uint8_t nch, uint8_t mode, uint16_t vel, uint16_t acc, uint16_t vel_step, uint32_t pos_step);

/*----------------------------------------------------------
 * 1.6 读取全部驱动参数命令
 *----------------------------------------------------------*/

/** @brief 读取系统状态参数 */
void Emm_V5_Read_System_State_Params(uint8_t addr);

/** @brief 读取驱动配置参数 */
void Emm_V5_Read_Motor_Conf_Params(uint8_t addr);


/*##########################################################
 *#
 *#  第二部分：多电机命令装载（MMCL_ 系列）
 *#  将命令帧写入 MMCL_cmd 缓冲区，
 *#  最后调用 Emm_V5_Multi_Motor_Cmd() 统一发送
 *#
 *##########################################################*/

/*----------------------------------------------------------
 * 2.1 触发动作命令 — 装载到多电机指令
 *----------------------------------------------------------*/
void Emm_V5_MMCL_Trig_Encoder_Cal(uint8_t addr);
void Emm_V5_MMCL_Reset_Motor(uint8_t addr);
void Emm_V5_MMCL_Reset_CurPos_To_Zero(uint8_t addr);
void Emm_V5_MMCL_Reset_Clog_Pro(uint8_t addr);
void Emm_V5_MMCL_Restore_Motor(uint8_t addr);

/*----------------------------------------------------------
 * 2.2 运动控制命令 — 装载到多电机指令
 *----------------------------------------------------------*/
void Emm_V5_MMCL_En_Control(uint8_t addr, bool state, bool snF);
void Emm_V5_MMCL_Vel_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, bool snF);
void Emm_V5_MMCL_Pos_Control(uint8_t addr, uint8_t dir, uint16_t vel, uint8_t acc, uint32_t clk, bool raF, bool snF);
void Emm_V5_MMCL_Stop_Now(uint8_t addr, bool snF);
void Emm_V5_MMCL_Synchronous_motion(uint8_t addr);

/*----------------------------------------------------------
 * 2.3 原点回零命令 — 装载到多电机指令
 *----------------------------------------------------------*/
void Emm_V5_MMCL_Origin_Set_O(uint8_t addr, bool svF);
void Emm_V5_MMCL_Origin_Trigger_Return(uint8_t addr, uint8_t o_mode, bool snF);
void Emm_V5_MMCL_Origin_Interrupt(uint8_t addr);
void Emm_V5_MMCL_Origin_Modify_Params(uint8_t addr, bool svF, uint8_t o_mode, uint8_t o_dir, uint16_t o_vel, uint32_t o_tm, uint16_t sl_vel, uint16_t sl_ma, uint16_t sl_ms, bool potF);

/*----------------------------------------------------------
 * 2.4 读取系统参数命令 — 装载到多电机指令
 *----------------------------------------------------------*/
void Emm_V5_MMCL_Auto_Return_Sys_Params_Timed(uint8_t addr, SysParams_t s, uint16_t time_ms);
void Emm_V5_MMCL_Read_Sys_Params(uint8_t addr, SysParams_t s);


#endif /* __ZDT_MOTOR_H__ */
