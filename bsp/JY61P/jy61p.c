/**
 * @file    jy61p.c
 * @brief   JY61P六轴IMU传感器驱动实现（UART中断接收 + 命令控制）
 *
 * @details 本驱动包含两大部分功能：
 *
 * 【一、数据接收】通过 UART RX 中断接收 JY61P 模块数据帧，
 *   ISR 内状态机逐字节解析，校验通过后更新数据结构。
 *   - 帧结构：帧头(0x55) + 类型码 + 8字节数据 + 校验和，共 11 字节
 *   - 帧类型：
 *     · 0x50 时间帧（Year, Month, Day, HH, MM, SS, ms）
 *     · 0x51 加速度帧（Ax, Ay, Az, T）
 *     · 0x52 角速度帧（Wx, Wy, Wz, T）
 *     · 0x53 角度帧（Roll, Pitch, Yaw, T）
 *   - 数据格式：小端序（低字节在前），有符号 16 位整数
 *   - 角度帧校验和算法与加速度/角速度帧不同，对角度帧做容错处理
 *
 * 【二、命令控制】通过 UART TX 向 JY61P 发送配置/校准命令。
 *   - 寄存器写入协议（5字节）：[0xFF, 0xAA, reg_addr, value_lo, value_hi]
 *   - 短命令协议（3字节）：[0xFF, 0xAA, cmd_byte]
 *   - 寄存器写入需要 解锁→写寄存器→保存 三步流程
 *
 * @note    依赖 uart.c 提供的 uart_send_buf() 发送命令
 */

#include "jy61p.h"
#include "ti_msp_dl_config.h"
#include "delay.h"

/* ======================== 帧定义 ======================== */
/*
 * JY61P 数据输出帧格式（每帧固定 11 字节）：
 *   [0]  帧头      = 0x55
 *   [1]  类型码    = 0x50/0x51/0x52/0x53
 *   [2~9] 数据载荷 = 4 个 int16 小端序
 *   [10] 校验和    = sum(buf[0..9]) & 0xFF
 */
#define JY61P_FRAME_HEADER   0x55U   /**< 帧头标识，所有帧的第一字节 */
#define JY61P_FRAME_TIME     0x50U   /**< 时间帧：年/月/日/时/分/秒/毫秒（1Hz 输出） */
#define JY61P_FRAME_ACC      0x51U   /**< 加速度帧：Ax, Ay, Az, T（100Hz 输出） */
#define JY61P_FRAME_GYRO     0x52U   /**< 角速度帧：Wx, Wy, Wz, T（100Hz 输出） */
#define JY61P_FRAME_ANGLE    0x53U   /**< 角度帧：Roll, Pitch, Yaw, T（100Hz 输出） */
#define JY61P_FRAME_LEN      11U     /**< 每帧固定长度：帧头(1)+类型(1)+数据(8)+校验(1) */

/* ======================== 命令协议 ======================== */
/*
 * JY61P 支持两种命令格式：
 *
 * 1. 寄存器写入协议（5字节，用于配置类命令）：
 *    [0xFF] [0xAA] [reg_addr] [value_lo] [value_hi]
 *    操作必须经过三步流程：
 *    ① 解锁：写入寄存器 0x69 = 0xB588（解除写保护）
 *    ② 写入：写入目标寄存器的 16 位值（小端序）
 *    ③ 保存：写入寄存器 0x00 = 0x0000（将配置保存到 EEPROM）
 *    每步之间需要 200ms 延时等待模块处理
 *
 * 2. 短命令协议（3字节，用于一次性校准命令）：
 *    [0xFF] [0xAA] [cmd_byte]
 *    无需解锁和保存，直接发送即可执行
 */

/* ======================== 状态机 ======================== */
/*
 * 状态机解析流程：
 *   STATE_WAIT_HEADER  --[收到 0x55]--> STATE_WAIT_TYPE
 *   STATE_WAIT_TYPE    --[合法类型码]--> STATE_RECV_DATA
 *                      --[非法类型码]--> STATE_WAIT_HEADER（丢弃）
 *   STATE_RECV_DATA    --[收满 11 字节]--> 校验+解析 --> STATE_WAIT_HEADER
 *   任何状态异常均回到 STATE_WAIT_HEADER 重新同步
 */

/**
 * @brief 帧解析状态机状态
 */
typedef enum {
    STATE_WAIT_HEADER = 0,  /**< 等待帧头 0x55，忽略所有非帧头字节 */
    STATE_WAIT_TYPE,        /**< 已收到帧头，等待类型码（0x50/0x51/0x52/0x53） */
    STATE_RECV_DATA,        /**< 已确认帧类型，接收剩余 9 字节数据+校验 */
} jy61p_state_t;

/* ======================== 内部变量 ======================== */
/*
 * 以下变量全部为文件级静态变量，仅在 jy61p.c 内部使用。
 * g_jy61p_data 为 volatile，因为由 ISR 写入、主循环读取，
 * 编译器不可优化其访问，必须每次都从内存读取。
 */

/** 串口号 → UART外设实例映射表（与 uart.c 中的保持一致，用于命令发送时查找外设） */
static UART_Regs *const g_uart_map[UART_PORT_MAX] = {
    [UART_PORT_0] = UART_0_INST,
    [UART_PORT_1] = UART_1_INST,
    [UART_PORT_2] = UART_2_INST,
    [UART_PORT_3] = UART_3_INST,
};

static volatile jy61p_data_t  g_jy61p_data;          /**< 全局数据实例 (ISR写入, 主循环读取, volatile防止编译器优化) */
static volatile uint8_t g_has_received = 0;         /**< 是否收到过有效数据帧 */
static volatile uint16_t g_rx_intr_cnt = 0;         /**< RX ISR 触发次数（用于自动检测） */
static jy61p_state_t g_state = STATE_WAIT_HEADER;   /**< 状态机当前状态，初始为等待帧头 */
static uint8_t       g_frame_buf[JY61P_FRAME_LEN];  /**< 当前帧缓冲区，存储完整的 11 字节帧数据 */
static uint8_t       g_frame_idx;                   /**< 当前帧已接收字节计数（0~11） */
static uint8_t       g_frame_type;                  /**< 当前帧类型码（0x50/0x51/0x52/0x53） */
static uint16_t      g_checksum;                    /**< 累加校验和（累加前 10 字节，取低 8 位与第 11 字节比较） */

/* ======================== 内部函数 ======================== */

/**
 * @brief  校验和验证
 * @param  type       帧类型（0x50/0x51/0x52/0x53）
 * @param  frame_buf  完整帧缓冲区（11字节）
 * @retval 1 校验通过, 0 校验失败
 * @note   加速度/角速度帧：sum(buf[0..9]) & 0xFF == buf[10]
 *         角度帧(0x53)：部分JY61P固件使用不同的校验算法，
 *         此处对角度帧做容错处理。
 */
static int jy61p_verify_checksum(uint8_t type, const uint8_t *frame_buf) {
    uint8_t sum = 0;
    for (int i = 0; i < JY61P_FRAME_LEN - 1; i++) {
        sum += frame_buf[i];
    }
    if (sum == frame_buf[JY61P_FRAME_LEN - 1]) {
        return 1;
    }
    /*
     * 角度帧(0x53)部分固件校验和算法不同：
     * 若标准累加校验失败，对角度帧做容错处理（放弃校验，直接使用数据）
     * 加速度/角速度帧仍严格校验
     */
    if (type == JY61P_FRAME_ANGLE) {
        return 1;  /* 角度帧容错，跳过校验 */
    }
    return 0;
}

/**
 * @brief  解析并保存时间数据（0x50 帧）
 * @param  frame_buf 完整帧缓冲区（11字节）
 * @note   帧数据布局：
 *         [2]=年(offset+2000) [3]=月 [4]=日 [5]=时 [6]=分 [7]=秒 [8..9]=毫秒
 *         本驱动仅提取时/分/秒，年/日/毫秒被忽略
 */
static void jy61p_parse_time(const uint8_t *frame_buf) {
    g_jy61p_data.hour   = frame_buf[5];
    g_jy61p_data.minute = frame_buf[6];
    g_jy61p_data.second = frame_buf[7];
    g_jy61p_data.time_updated = 1;
}

/**
 * @brief  解析并保存加速度数据（0x51 帧）
 * @param  frame_buf 完整帧缓冲区（11字节）
 * @note   帧数据布局：[2..3]=Ax, [4..5]=Ay, [6..7]=Az, [8..9]=T（温度）
 *         小端序有符号 16 位原始值在此直接换算：
 *         acc(g) = (int16_t)raw / 32768.0f × 16.0f
 */
static void jy61p_parse_acc(const uint8_t *frame_buf) {
    g_jy61p_data.acc_x = (int16_t)((uint16_t)frame_buf[3] << 8 | frame_buf[2]) / 32768.0f * 16.0f;
    g_jy61p_data.acc_y = (int16_t)((uint16_t)frame_buf[5] << 8 | frame_buf[4]) / 32768.0f * 16.0f;
    g_jy61p_data.acc_z = (int16_t)((uint16_t)frame_buf[7] << 8 | frame_buf[6]) / 32768.0f * 16.0f;
    g_jy61p_data.acc_updated = 1;
}

/**
 * @brief  解析并保存角速度数据（0x52 帧）
 * @param  frame_buf 完整帧缓冲区（11字节）
 * @note   帧数据布局：[2..3]=Wx, [4..5]=Wy, [6..7]=Wz, [8..9]=T（温度）
 *         小端序有符号 16 位原始值在此直接换算：
 *         gyro(°/s) = (int16_t)raw / 32768.0f × 2000.0f
 */
static void jy61p_parse_gyro(const uint8_t *frame_buf) {
    g_jy61p_data.gyro_x = (int16_t)((uint16_t)frame_buf[3] << 8 | frame_buf[2]) / 32768.0f * 2000.0f;
    g_jy61p_data.gyro_y = (int16_t)((uint16_t)frame_buf[5] << 8 | frame_buf[4]) / 32768.0f * 2000.0f;
    g_jy61p_data.gyro_z = (int16_t)((uint16_t)frame_buf[7] << 8 | frame_buf[6]) / 32768.0f * 2000.0f;
    g_jy61p_data.gyro_updated = 1;
}

/**
 * @brief  解析并保存欧拉角数据（0x53 帧）
 * @param  frame_buf 完整帧缓冲区（11字节）
 * @note   帧数据布局：[2..3]=Roll, [4..5]=Pitch, [6..7]=Yaw, [8..9]=T
 *         小端序有符号 16 位原始值在此直接换算：
 *         angle(°) = (int16_t)raw / 32768.0f × 180.0f
 *         范围：Roll/Pitch = ±180°，Yaw = ±180°
 */
static void jy61p_parse_angle(const uint8_t *frame_buf) {
    g_jy61p_data.angle_roll  = (int16_t)((uint16_t)frame_buf[3] << 8 | frame_buf[2]) / 32768.0f * 180.0f;
    g_jy61p_data.angle_pitch = (int16_t)((uint16_t)frame_buf[5] << 8 | frame_buf[4]) / 32768.0f * 180.0f;
    g_jy61p_data.angle_yaw   = (int16_t)((uint16_t)frame_buf[7] << 8 | frame_buf[6]) / 32768.0f * 180.0f;
    g_jy61p_data.angle_updated = 1;
}

/**
 * @brief  处理一帧完整数据（校验后分发到对应解析函数）
 * @param  type      帧类型码（0x50/0x51/0x52/0x53）
 * @param  frame_buf 完整帧缓冲区（11字节）
 * @note   流程：先校验 → 校验通过后根据类型码分发到对应解析函数
 *         校验失败的帧被静默丢弃，不更新任何数据
 */
static void jy61p_process_frame(uint8_t type, const uint8_t *frame_buf) {
    if (!jy61p_verify_checksum(type, frame_buf)) {
        return;  /* 校验失败，丢弃 */
    }
    g_has_received = 1;  /* 标记已收到有效数据 */
    switch (type) {
        case JY61P_FRAME_TIME:  jy61p_parse_time(frame_buf);  break;
        case JY61P_FRAME_ACC:   jy61p_parse_acc(frame_buf);   break;
        case JY61P_FRAME_GYRO:  jy61p_parse_gyro(frame_buf);  break;
        case JY61P_FRAME_ANGLE: jy61p_parse_angle(frame_buf); break;
        default: break;
    }
}

/* ======================== 公开接口 ======================== */
/*
 * 以下函数供外部调用：
 * - jy61p_get_data()   获取只读数据指针，主循环中读取显示
 * - jy61p_clear_flags() 清除更新标志，表示已处理完当前数据
 * - jy61p_init()       初始化 NVIC 中断，必须在 SYSCFG_DL_init() 后调用
 * - jy61p_poll()       已废弃，中断模式下为空实现
 */

/**
 * @brief  获取 JY61P 数据指针（只读访问）
 * @retval 指向内部 volatile 数据结构的 const 指针
 * @note   返回值指向的数据可能被 ISR 随时更新，
 *         建议在读取后尽快使用，并配合 clear_flags 判断新数据
 */
const jy61p_data_t *jy61p_get_data(void) {
    return (const jy61p_data_t *)&g_jy61p_data;
}

/**
 * @brief  清除所有数据更新标志
 * @note   在主循环中读取完数据后调用，用于标记“已处理”。
 *         下次 ISR 更新数据时会重新置位对应标志。
 *         典型用法：if (imu->angle_updated) { 显示; jy61p_clear_flags(); }
 */
void jy61p_clear_flags(void) {
    g_jy61p_data.acc_updated   = 0;
    g_jy61p_data.gyro_updated  = 0;
    g_jy61p_data.angle_updated = 0;
    g_jy61p_data.time_updated  = 0;
}

/* ======================== 内部：状态机处理单字节 ======================== */

/**
 * @brief  将一个字节喂入状态机（ISR 和轮询共用）
 * @param  byte 从 UART FIFO 读取到的字节
 * @note   状态机工作流程：
 *         1. WAIT_HEADER: 只关心 0x55，其他字节全部丢弃
 *         2. WAIT_TYPE:   检查类型码是否合法（0x50~0x53），非法则回到 WAIT_HEADER
 *         3. RECV_DATA:   逐字节存入 frame_buf，收满 11 字节后调用 process_frame 解析
 */
static void jy61p_process_byte(uint8_t byte) {
    switch (g_state) {
        case STATE_WAIT_HEADER:
            /* 只关心帧头 0x55，其他字节全部丢弃 */
            if (byte == JY61P_FRAME_HEADER) {
                g_frame_buf[0] = byte;
                g_frame_idx = 1;
                g_checksum  = byte;
                g_state = STATE_WAIT_TYPE;
            }
            break;

        case STATE_WAIT_TYPE:
            /* 收到帧头后的第二个字节为类型码，判断是否为合法帧类型 */
            g_frame_buf[1] = byte;
            g_frame_type   = byte;
            g_checksum    += byte;
            g_frame_idx    = 2;
            if (byte == JY61P_FRAME_TIME ||
                byte == JY61P_FRAME_ACC  ||
                byte == JY61P_FRAME_GYRO ||
                byte == JY61P_FRAME_ANGLE) {
                g_state = STATE_RECV_DATA;
            } else {
                g_state = STATE_WAIT_HEADER;
            }
            break;

        case STATE_RECV_DATA:
            /* 逐字节存入帧缓冲区，并累加校验和 */
            g_frame_buf[g_frame_idx] = byte;
            g_checksum += byte;
            g_frame_idx++;
            if (g_frame_idx >= JY61P_FRAME_LEN) {
                /* 一帧接收完毕（11字节），交给 process_frame 校验并解析 */
                jy61p_process_frame(g_frame_type, g_frame_buf);
                g_state = STATE_WAIT_HEADER;
            }
            break;

        default:
            g_state = STATE_WAIT_HEADER;
            break;
    }
}

/* ======================== 初始化 & 轮询兼容 ======================== */

/**
 * @brief  JY61P 初始化，启用 UART_3 的 NVIC 中断
 * @note   调用顺序：SYSCFG_DL_init() → OLED_Init() → jy61p_init()
 *         SysConfig 生成的 SYSCFG_DL_UART_3_init() 已在外设级启用了 RX 中断
 *         （DL_UART_Main_enableInterrupt），这里只需启用 NVIC 即可让 ISR 真正触发。
 *         如果不调用此函数，RX 中断标志会被置位但 ISR 不会执行。
 */
/**
  * @brief    JY61P 串口初始化（使能 UART RX 中断）
  * @retval   无
  */
void jy61p_init(void) {
    g_has_received = 0;
    g_rx_intr_cnt  = 0;
    NVIC_EnableIRQ(UART_3_INST_INT_IRQN);
}

/**
 * @brief  自动检测 IMU 模块型号
 * @retval 检测结果（JY61P / JY60 / NONE）
 * @note   检测逻辑：
 *         - 收到有效帧 → JY61P_MODEL_JY61P
 *         - 无有效帧 + ISR 未被触发 → RX 被驱动为高 → JY61P_MODEL_JY60
 *         - 无有效帧 + ISR 被触发过 → RX 噪声 → JY61P_MODEL_NONE
 */
jy61p_model_t jy61p_detect(void) {
    if (g_has_received) {
        return JY61P_MODEL_JY61P;
    }
    if (g_rx_intr_cnt == 0) {
        return JY61P_MODEL_JY60;   /* RX 引脚被驱动，无噪声 → JY60 在线 */
    }
    return JY61P_MODEL_NONE;        /* RX 有噪声但无有效帧 → 悬空 */
}

/**
 * @brief  JY61P 轮询接口（已废弃，保留兼容）
 * @note   中断模式下此函数为空实现，数据由 UART3_IRQHandler 自动接收。
 *         保留此函数是为了旧代码调用时不会报错。
 */
void jy61p_poll(void) {
    /* 中断模式下为空实现，数据由 ISR 自动接收解析 */
}

/* ======================== UART_3 中断服务函数 ======================== */
/*
 * 当 UART_3 的 RX FIFO 有数据时触发此 ISR。
 * SysConfig 中配置的 RX FIFO 阈值为 "ONE_ENTRY"，
 * 即每收到 1 个字节就触发中断。
 * ISR 内循环读空 FIFO，每字节喂入状态机解析。
 */

/**
 * @brief  UART_3 RX 中断服务函数
 * @note   函数名由 UART_3_INST_IRQHandler 宏展开为 UART3_IRQHandler，
 *         与启动文件中的中断向量表一致。
 */
/**
  * @brief    JY61P UART RX 中断服务函数
  * @note     逐字节读取 FIFO 并送入协议解析状态机
  * @retval   无
  */
void UART_3_INST_IRQHandler(void) {
    UART_Regs *uart = UART_3_INST;

    g_rx_intr_cnt++;  /* 记录 ISR 触发（用于自动检测） */

    /*
     * 循环读空 RX FIFO：
     * 虽然阈值设为 1 字节触发中断，但 ISR 执行期间可能又有新字节到达，
     * 因此必须循环读取直到 FIFO 为空，避免漏数据。
     */
    while (!DL_UART_Main_isRXFIFOEmpty(uart)) {
        uint8_t byte = DL_UART_Main_receiveData(uart);
        jy61p_process_byte(byte);
    }
}

/* ======================== 校准/控制命令实现 ======================== */
/*
 * JY61P 配置命令必须经过 解锁→写入→保存 三步流程，
 * 校准命令使用 3 字节短协议，无需解锁和保存。
 *
 * 寄存器写入三步流程：
 *   1. 解锁：发送 FF AA 69 88 B5（解除寄存器写保护）→ 延时 200ms
 *   2. 写入：发送 FF AA reg val_lo val_hi（写目标寄存器）→ 延时 200ms
 *   3. 保存：发送 FF AA 00 00 00（将配置写入 EEPROM 掉电保存）→ 延时 200ms
 */

/** 解锁命令：向寄存器 0x69 写入 0xB588，解除写保护 */
static const uint8_t JY61P_UNLOCK_CMD[] = {0xFF, 0xAA, 0x69, 0x88, 0xB5};
/** 保存命令：向寄存器 0x00 写入 0x0000，触发 EEPROM 保存 */
static const uint8_t JY61P_SAVE_CMD[]  = {0xFF, 0xAA, 0x00, 0x00, 0x00};

/**
 * @brief  向 JY61P 写入一个寄存器值（含解锁+保存流程）
 * @param  reg_addr  寄存器地址
 * @param  value     16位寄存器值（小端序发送）
 * @param  extra_ms  写入后额外等待的毫秒数（校准类命令需要较长等待）
 * @note   操作流程：解锁 → 延时200ms → 写寄存器 → 延时(200+extra)ms → 保存 → 延时200ms
 */
static void jy61p_write_reg(uint8_t reg_addr, uint16_t value, uint32_t extra_ms) {
    uint8_t cmd[5] = {0xFF, 0xAA, reg_addr, (uint8_t)(value & 0xFF), (uint8_t)(value >> 8)};

    uart_send_buf(JY61P_UART_PORT, JY61P_UNLOCK_CMD, 5);  /* 解锁 */
    delay_ms(200);

    uart_send_buf(JY61P_UART_PORT, cmd, 5);                /* 写寄存器 */
    delay_ms(200 + extra_ms);

    uart_send_buf(JY61P_UART_PORT, JY61P_SAVE_CMD, 5);     /* 保存 */
    delay_ms(200);
}

/**
 * @brief  向 JY61P 发送 3 字节短命令（旧版协议，无寄存器地址）
 * @param  cmd_byte  命令字节（如 0x67=加速度校准, 0x52=Z轴归零）
 * @note   部分校准命令使用 3 字节格式：[0xFF, 0xAA, cmd_byte]
 */
static void jy61p_send_short_cmd(uint8_t cmd_byte) {
    uint8_t cmd[3] = {0xFF, 0xAA, cmd_byte};
    uart_send_buf(JY61P_UART_PORT, cmd, 3);
}

void jy61p_calibrate_acc(void) {
    /*
     * 加速度校准（3字节短命令 0x67）：
     * 将模块水平放置后调用，校准过程中保持绝对静止。
     * 模块会自动计算加速度零偏并保存到内部 EEPROM，
     * 校准完成后模块重启，约需 2~3 秒。
     */
    jy61p_send_short_cmd(0x67);
    delay_ms(3000);
}

void jy61p_calibrate_gyro(void) {
    /*
     * 陀螺仪归零（3字节短命令 0x52）：
     * 将模块静止放置后调用，校准过程中绝对不可移动/振动。
     * 模块会自动计算陀螺仪零偏并保存，约需 2~3 秒。
     * 上电后若模块静止，模块也会自动进行零偏校准。
     */
    jy61p_send_short_cmd(0x52);
    delay_ms(3000);
}

void jy61p_calibrate_mag(void) {
    /*
     * 磁场校准（3字节短命令 0x68）：
     * 发送命令后需在 10 秒内将模块在空间中旋转 360°。
     * JY61P 若不带磁力计（纯六轴模块）则此命令无效。
     */
    jy61p_send_short_cmd(0x68);
    delay_ms(100);
}

void jy61p_reset_angle_xy(void) {
    /*
     * XY轴归零（寄存器写入协议）：
     * 向寄存器 0x01 写入值 0x0008，以当前姿态为参考点，
     * 使 Roll/Pitch 输出变为 0。
     * 完整流程：解锁 → 写 0x01=0x0008 → 保存
     */
    jy61p_write_reg(0x01, 0x0008, 0);
}

void jy61p_reset_angle_z(void) {
    /*
     * Z轴归零（寄存器写入协议）：
     * 向寄存器 0x01 写入值 0x0004，使 Yaw 输出变为 0。
     * 注意：仅在六轴算法模式下有效。
     * 九轴模式下 Yaw 基于地磁绝对方向，无法归零。
     */
    jy61p_write_reg(0x01, 0x0004, 0);
}

void jy61p_reset_angle(void) {
    /*
     * 全轴角度归零：
     * ① JY61 XY 归零 → ② JY61 Z 归零 → ③ JY60 Z 归零
     * 各命令之间间隔 50ms，确保模块处理完毕。
     */
    jy61p_write_reg(0x01, 0x0008, 0);   /* JY61 XY 归零 */
    delay_ms(50);
    jy61p_write_reg(0x01, 0x0004, 0);   /* JY61 Z 归零 */
    delay_ms(50);
    jy61p_send_short_cmd(0x52);         /* JY60 Z 归零 (FF AA 52) */
}
