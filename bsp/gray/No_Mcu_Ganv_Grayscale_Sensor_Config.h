/**
 * @file    No_Mcu_Ganv_Grayscale_Sensor_Config.h
 * @brief   8通道灰度传感器硬件抽象层配置与接口声明
 *
 * @details 本头文件定义了灰度传感器驱动的硬件抽象层（HAL）：
 *   - 地址线 GPIO 切换宏（3位通道选择）
 *   - ADC 采样接口宏（用户可替换为平台对应函数）
 *   - 驱动函数声明
 *
 * 通道选择原理：
 *   3 根地址线（Address_0/1/2）组合产生 0~7 共 8 个通道，
 *   通过多路复用器切换到共用 ADC 进行采集。
 */
#ifndef NO_MCU_GANV_GRAYSCALE_SENSOR_CONFIG_H_
#define NO_MCU_GANV_GRAYSCALE_SENSOR_CONFIG_H_

#include "ti_msp_dl_config.h"
#include "ADC.h"
#include "uart.h"



/* ======================== 硬件抽象层配置 ======================== */

/**
 * @brief  地址线 GPIO 切换宏（3位通道选择）
 * @param  i  非零=输出高电平, 零=输出低电平
 * @note   每个宏控制一根地址线的 GPIO 输出：
 *         - Switch_Address_0 对应 bit0（通道选择最低位）
 *         - Switch_Address_1 对应 bit1
 *         - Switch_Address_2 对应 bit2（通道选择最高位）
 *         3 位组合可寻址 0~7 共 8 个传感器通道
 */
#define Switch_Address_0(i) ((i) ? \
    (DL_GPIO_setPins(Gray_Address_PORT, Gray_Address_PIN_0_PIN)) : \
    (DL_GPIO_clearPins(Gray_Address_PORT, Gray_Address_PIN_0_PIN)))

#define Switch_Address_1(i) ((i) ? \
    (DL_GPIO_setPins(Gray_Address_PORT, Gray_Address_PIN_1_PIN)) : \
    (DL_GPIO_clearPins(Gray_Address_PORT, Gray_Address_PIN_1_PIN)))

#define Switch_Address_2(i) ((i) ? \
    (DL_GPIO_setPins(Gray_Address_PORT, Gray_Address_PIN_2_PIN)) : \
    (DL_GPIO_clearPins(Gray_Address_PORT, Gray_Address_PIN_2_PIN)))

/**
 * @brief  ADC 采样接口宏
 * @note   用户根据实际单片机平台替换为对应的 ADC 读取函数。
 *         当前实现调用 ADC.c 中的 adc_getValue()，
 *         执行单次转换并轮询等待结果。
 */
#define Get_adc_of_user() adc_getValue()

#ifdef __cplusplus
extern "C" {
#endif

/* ======================== 函数声明 ======================== */

/**
 * @brief  采集 8 通道 ADC 均值、二值化并计算循迹偏差
 * @param  result    输出数组（8 元素），先填 ADC 均值，后被二值化覆盖
 * @param  threshold 阈值数组（8 元素），用于二值化比较
 * @retval 循迹偏差值（正=右偏, 负=左偏, 0=居中, 99=全黑）
 */
int Get_Analog_value(unsigned short *result, unsigned short *threshold);

/**
 * @brief  将 ADC 原始值数组二值化为 0/1
 * @param  result 输入/输出数组（8 元素）
 * @param  black  阈值数组（8 元素）
 */
void get_digit(unsigned short *result, unsigned short *black);

/**
 * @brief  根据二值化传感器状态计算循迹偏差值
 * @param  result 二值化后的传感器状态数组（8 元素）
 * @retval 偏差值（正=右偏, 负=左偏, 0=居中, 99=全黑）
 */
int GRAY_GetError(unsigned short *result);

#ifdef __cplusplus
}
#endif

#endif /* NO_MCU_GANV_GRAYSCALE_SENSOR_CONFIG_H_ */