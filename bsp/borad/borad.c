/**
 * @file    borad.c
 * @brief   板级外设驱动（LED、蜂鸣器、按键）
 * @note    通过 GPIO 控制 3 路 LED、1 路蜂鸣器，读取 4 路按键状态
 */

#include "borad.h"
#include "ti_msp_dl_config.h"

/**
  * @brief    设置 LED 亮灭
  * @param    led   ：LED 编号（1~3）
  * @param    state ：状态
  *                   = 0 : 熄灭
  *                   !=0 : 点亮
  * @retval   无
  */
void led_set(uint8_t led, uint8_t state)
{
    if (led == 1)
    {
        if (state)
            DL_GPIO_setPins(borad_led_1_PORT, borad_led_1_PIN);
        else
            DL_GPIO_clearPins(borad_led_1_PORT, borad_led_1_PIN);
    }
    else if (led == 2)
    {
        if (state)
            DL_GPIO_setPins(borad_led_2_PORT, borad_led_2_PIN);
        else
            DL_GPIO_clearPins(borad_led_2_PORT, borad_led_2_PIN);
    }
    else if (led == 3)
    {
        if (state)
            DL_GPIO_setPins(borad_led_3_PORT, borad_led_3_PIN);
        else
            DL_GPIO_clearPins(borad_led_3_PORT, borad_led_3_PIN);
    }
}

/**
  * @brief    翻转 LED 状态
  * @param    led   ：LED 编号（1~3）
  * @retval   无
  */
void led_toggle(uint8_t led)
{
    if (led == 1)
    {
        DL_GPIO_togglePins(borad_led_1_PORT, borad_led_1_PIN);
    }
    else if (led == 2)
    {
        DL_GPIO_togglePins(borad_led_2_PORT, borad_led_2_PIN);
    }
    else if (led == 3)
    {
        DL_GPIO_togglePins(borad_led_3_PORT, borad_led_3_PIN);
    }
}

/**
  * @brief    设置蜂鸣器
  * @param    state ：状态
  *                   = 0 : 关闭
  *                   !=0 : 开启
  * @retval   无
  */
void buzzer_set(uint8_t state)
{
    if (state)
        DL_GPIO_setPins(borad_buzzer_PORT, borad_buzzer_PIN);
    else
        DL_GPIO_clearPins(borad_buzzer_PORT, borad_buzzer_PIN);
}

/**
  * @brief    读取按键状态（归一化为 0/1）
  * @param    key   ：按键编号（1~4）
  * @retval   0  : 按下（低电平有效）
  * @retval   1  : 未按下
  */
uint8_t key_get(uint8_t key)
{
    if (key == 1)
    {
        return (DL_GPIO_readPins(borad_key_1_PORT, borad_key_1_PIN) != 0U) ? 1U : 0U;
    }
    else if (key == 2)
    {
        return (DL_GPIO_readPins(borad_key_2_PORT, borad_key_2_PIN) != 0U) ? 1U : 0U;
    }
    else if (key == 3)
    {
        return (DL_GPIO_readPins(borad_key_3_PORT, borad_key_3_PIN) != 0U) ? 1U : 0U;
    }
    else if (key == 4)
    {
        return (DL_GPIO_readPins(borad_key_4_PORT, borad_key_4_PIN) != 0U) ? 1U : 0U;
    }
    else
    {
        return 0U;
    }
}
