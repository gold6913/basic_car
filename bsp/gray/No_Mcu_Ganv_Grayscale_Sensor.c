/**
 * @file    No_Mcu_Ganv_Grayscale_Sensor.c
 * @brief   8通道灰度传感器驱动实现（多路复用ADC采集 + 二值化 + 循迹偏差计算）
 *
 * @details 本驱动用于控制 8 通道灰度传感器模组（如冈谷 TCRT5000 阵列），
 *          通过 3 位地址线进行通道切换，共用 1 路 ADC 完成模拟量采集。
 *
 * 【一、模拟量采集 — Get_Analog_value()】
 *   - 通过 3 根地址线（Address_0/1/2）的 GPIO 组合选择 0~7 通道
 *   - 每个通道进行 30 次 ADC 采样取均值，实现均值滤波
 *   - 通道切换后需延时等待模拟开关稳定（约 10ms）
 *   - 采集完成后自动调用二值化，再计算偏差值返回
 *   - 返回值：循迹偏差（连续值，约 -35 ~ +35，正=右偏, 负=左偏），丢线时保持上一次有效值
 *
 * 【二、二值化 — get_digit()】
 *   - 将每个通道的 ADC 原始值与阈值（threshold/black）逐通道比较
 *   - ADC < 阈值 → 0（检测到黑线）
 *   - ADC >= 阈值 → 1（检测到白面）
 *   - 二值化结果直接覆盖原 result 数组
 *
 * 【三、偏差计算 — GRAY_GetError()】
 *   - 基于二值化后的 8 位传感器状态，用加权平均法计算循迹偏差
 *   - 统计检测到黑线的通道位置（0~7），计算平均位置
 *   - 偏差 = (平均位置 - 3.5) × 10，正=右偏, 负=左偏, 0=居中
 *   - 所有通道均未检测到黑线 → 保持上一次有效偏差值
 *
 * @note    依赖 ADC.c 的 adc_getValue() 完成单次 ADC 转换
 *          依赖 Config.h 中的 Switch_Address_x() 宏完成通道切换
 */

#include "No_Mcu_Ganv_Grayscale_Sensor_Config.h"
#include <stdio.h>    /* sprintf */

/* ======================== 偏差计算说明 ======================== */
/*
 * 采用加权平均法，不再使用离散等级判断：
 *   - 统计检测到黑线的通道位置（0~7），计算平均位置
 *   - 偏差 = (平均位置 - 3.5) × 10，范围约 -35 ~ +35
 *   - 正值为右偏，负值为左偏，0 为居中
 *   - 全部通道为白面（丢线）→ 保持上一次有效值
 *
 * 传感器通道布局（从左到右）：
 *   [7] [6] [5] [4] [3] [2] [1] [0]
 *        ← 左偏         右偏 →
 */

/* ======================== 公开接口 ======================== */

/**
 * @brief  采集 8 通道灰度模拟量，二值化后计算循迹偏差
 * @param  result    输出数组（8 元素），先填入 ADC 均值，后被二值化覆盖为 0/1
 * @param  threshold 阈值数组（8 元素），ADC < 阈值判为黑线(0)，否则白面(1)
 * @retval 循迹偏差值（正=右偏, 负=左偏, 0=居中, 丢线时保持上一次有效值）
 * @note   完整流程：
 *         ① 遍历 8 通道 → 地址线切换 → 30 次 ADC 采样取均值 → 存入 result
 *         ② 调用 get_digit() 对 result 进行二值化（0/1）
 *         ③ 调用 GRAY_GetError() 根据二值化结果计算偏差
 *         ④ 返回偏差值供 PID/控制算法使用
 */
int Get_Analog_value(unsigned short *result, unsigned short *threshold)
{
    unsigned char i, j;
    unsigned int Anolag = 0;

    /* 遍历 8 个传感器通道（3 位地址线组合 000~111） */
    for (i = 0; i < 8; i++) {
        /*
         * 通过地址线组合切换多路复用器通道（注意取反逻辑）：
         *   Address_0 对应 bit0，Address_1 对应 bit1，Address_2 对应 bit2
         *   取反是因为硬件上地址线低电平有效
         */
        Switch_Address_0(!(i & 0x01));
        Switch_Address_1(!(i & 0x02));
        Switch_Address_2(!(i & 0x04));

        /* 通道切换后等待模拟开关稳定（约 10ms @ 32MHz） */
        delay_cycles(32000000 / 100000 * 10);

        /*
         * 每个通道采集 30 次 ADC 值进行均值滤波：
         * 累加 30 次采样结果，最后除以采样次数得到平均值，
         * 以抑制 ADC 随机噪声，提高信号稳定性
         */
        for (j = 0; j < 30; j++) {
            Anolag += Get_adc_of_user();
        }

        result[7 - i] = Anolag / j;  /* 注意索引翻转：通道 i 的数据存入 result[7-i]（匹配物理排列） */
        Anolag = 0;                  /* 重置累加器，为下一通道做准备 */
    }
    /* 二值化：将 ADC 原始均值转换为 0（黑线）/ 1（白面） */
    get_digit(result, threshold);

    /* 根据二值化结果计算循迹偏差并返回 */
    return GRAY_GetError(result);
}

/**
 * @brief  仅采集 8 通道 ADC 模拟量均值（不二值化），供调试/校准使用
 * @param  result 输出数组（8 元素），存入各通道原始 ADC 均值
 */
void Get_Analog_Raw(unsigned short *result)
{
    unsigned char i, j;
    unsigned int Anolag = 0;

    for (i = 0; i < 8; i++) {
        Switch_Address_0(!(i & 0x01));
        Switch_Address_1(!(i & 0x02));
        Switch_Address_2(!(i & 0x04));

        delay_cycles(32000000 / 100000 * 10);

        for (j = 0; j < 30; j++) {
            Anolag += Get_adc_of_user();
        }

        result[7 - i] = Anolag / j;
        Anolag = 0;
    }
}

/**
 * @brief  将 ADC 原始值数组二值化为 0/1
 * @param  result 输入/输出数组（8 元素），输入为 ADC 均值，输出被覆盖为 0 或 1
 * @param  black  阈值数组（8 元素），每个通道独立的比较阈值
 * @note   判断逻辑：
 *         - result[i] < black[i] → 0（检测到黑线，反射光弱，ADC 值低）
 *         - result[i] >= black[i] → 1（检测到白面，反射光强，ADC 值高）
 *         二值化结果直接覆盖原数组，节省内存
 */
void get_digit(unsigned short *result, unsigned short *black)
{
    unsigned short i;
    for (i = 0; i < 8; i++) {
        if (result[i] < black[i])
            result[i] = 0;   /* 黑线 */
        else
            result[i] = 1;   /* 白面 */
    }
    
}

/**
 * @brief  根据二值化传感器状态，用加权平均法计算循迹偏差值
 * @param  result 二值化后的传感器状态数组（8 元素，0=黑线, 1=白面）
 * @retval 循迹偏差值（正=右偏, 负=左偏, 0=居中, 丢线时保持上一次有效值）
 * @note   算法说明：
 *         - 遍历 8 通道，统计检测到黑线（值为 0）的通道位置和数量
 *         - 计算黑线通道的加权平均位置（0~7），中心位置为 3.5
 *         - 偏差 = (平均位置 - 3.5) × 10，得到连续偏差值（约 -35 ~ +35）
 *         - 全部通道为白面（丢线）时，保持上一次有效偏差值
 *
 *         传感器物理排列与数组索引对照：
 *         [7]=最左  [6]  [5]  [4]=中心左  [3]=中心右  [2]  [1]  [0]=最右
 */
int GRAY_GetError(unsigned short *result)
{
    uint8_t i;
    int32_t pos_sum = 0;       /**< 检测到黑线的通道位置累加和 */
    int32_t black_cnt = 0;     /**< 检测到黑线的通道数量 */
    static int last_error = 0; /**< 上一次有效偏差值（丢线时保持） */

    /* 遍历 8 通道，统计检测到黑线的通道位置和数量 */
    for (i = 0; i < 8; i++) {
        if (result[i] == 0) {   /* 0 = 检测到黑线 */
            pos_sum += i;       /* 累加通道位置（0~7） */
            black_cnt++;
        }
    }

    /* 全部通道为白面（丢线），保持上一次有效偏差值 */
    if (black_cnt == 0)
        return last_error;

    /*
     * 加权平均偏差计算：
     *   平均位置 = pos_sum / black_cnt       （范围 0~7）
     *   中心位置 = 3.5（通道 3 和 4 之间）
     *   偏差 = (平均位置 - 3.5) × 10
     *        = (pos_sum × 10 / black_cnt) - 35
     *
     *   示例：通道 [4] 黑 → (4×10/1)-35 =  5（轻微右偏）
     *         通道 [3][4] 黑 → (7×10/2)-35 =  0（居中）
     *         通道 [7][6][5] 黑 → (18×10/3)-35 = 25（严重左偏）
     */
    last_error = (int32_t)((pos_sum * 10) / black_cnt) - 35;
    return last_error;
}