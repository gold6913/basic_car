/**
 * @file    tb6612.h
 * @brief   TB6612 双路电机驱动芯片接口声明
 */

#ifndef TB6612_H
#define TB6612_H

#include "ti_msp_dl_config.h"

void motor_set(float speedA, float speedB, uint8_t move);

#endif /* TB6612_H */