/**
 * @file    ADC.c
 * @brief   ADC 单通道采集驱动（阻塞式）
 * @note    每次调用启动一次 ADC 转换并等待完成，适用于低速轮询场景
 */

#include "ADC.h"

/**
  * @brief    触发一次 ADC 单通道采集（阻塞等待）
  * @retval   12 位 ADC 原始值（0~4095）
  * @note     内部完成：清除中断标志 → 使能 → 启动 → 等待转换完成 → 读取 → 停止 → 禁能
  */
unsigned int adc_getValue(void)
{
    unsigned int gAdcResult = 0;

    /* 清除旧的结果标志，防止残留标志导致误判 */
    DL_ADC12_clearInterruptStatus(ADC12_0_INST, DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);

    /* 使能ADC转换 */
    DL_ADC12_enableConversions(ADC12_0_INST);
    /* 软件触发ADC开始转换 */
    DL_ADC12_startConversion(ADC12_0_INST);

    /* 等待 MEM0 结果加载完成（单次转换完成标志） */
    while (DL_ADC12_getPendingInterrupt(ADC12_0_INST) != DL_ADC12_IIDX_MEM0_RESULT_LOADED);

    /* 获取数据 */
    gAdcResult = DL_ADC12_getMemResult(ADC12_0_INST, ADC12_0_ADCMEM_ch0);

    /* 停止转换（退出 REPEAT 模式），防止持续转换影响下次采样 */
    DL_ADC12_stopConversion(ADC12_0_INST);
    DL_ADC12_disableConversions(ADC12_0_INST);

    return gAdcResult;
}