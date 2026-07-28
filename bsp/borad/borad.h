/**
 * @file    borad.h
 * @brief   板级外设驱动声明（LED、蜂鸣器、按键）
 */

#ifndef __BORAD_H__
#define __BORAD_H__

#include <stdint.h>

void led_set(uint8_t led, uint8_t state);
void led_toggle(uint8_t led);
void buzzer_set(uint8_t state);
uint8_t key_get(uint8_t key);

#endif /* __BORAD_H__ */
