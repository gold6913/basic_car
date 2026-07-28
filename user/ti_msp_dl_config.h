/*
 * Copyright (c) 2023, Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ============ ti_msp_dl_config.h =============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  DO NOT EDIT - This file is generated for the MSPM0G350X
 *  by the SysConfig tool.
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X
#define CONFIG_MSPM0G3507

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform all required MSP DL initialization
 *
 *  This function should be called once at a point before any use of
 *  MSP DL.
 */


/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)



#define CPUCLK_FREQ                                                     32000000



/* Defines for motor */
#define motor_INST                                                         TIMA1
#define motor_INST_IRQHandler                                   TIMA1_IRQHandler
#define motor_INST_INT_IRQN                                     (TIMA1_INT_IRQn)
#define motor_INST_CLK_FREQ                                              4000000
/* GPIO defines for channel 0 */
#define GPIO_motor_C0_PORT                                                 GPIOA
#define GPIO_motor_C0_PIN                                         DL_GPIO_PIN_17
#define GPIO_motor_C0_IOMUX                                      (IOMUX_PINCM39)
#define GPIO_motor_C0_IOMUX_FUNC                     IOMUX_PINCM39_PF_TIMA1_CCP0
#define GPIO_motor_C0_IDX                                    DL_TIMER_CC_0_INDEX
/* GPIO defines for channel 1 */
#define GPIO_motor_C1_PORT                                                 GPIOA
#define GPIO_motor_C1_PIN                                         DL_GPIO_PIN_16
#define GPIO_motor_C1_IOMUX                                      (IOMUX_PINCM38)
#define GPIO_motor_C1_IOMUX_FUNC                     IOMUX_PINCM38_PF_TIMA1_CCP1
#define GPIO_motor_C1_IDX                                    DL_TIMER_CC_1_INDEX



/* Defines for OLED_refresh */
#define OLED_refresh_INST                                                (TIMG6)
#define OLED_refresh_INST_IRQHandler                            TIMG6_IRQHandler
#define OLED_refresh_INST_INT_IRQN                              (TIMG6_INT_IRQn)
#define OLED_refresh_INST_LOAD_VALUE                                     (3999U)
/* Defines for encoder */
#define encoder_INST                                                     (TIMG0)
#define encoder_INST_IRQHandler                                 TIMG0_IRQHandler
#define encoder_INST_INT_IRQN                                   (TIMG0_INT_IRQn)
#define encoder_INST_LOAD_VALUE                                         (39999U)



/* Defines for UART_0 */
#define UART_0_INST                                                        UART0
#define UART_0_INST_FREQUENCY                                            4000000
#define UART_0_INST_IRQHandler                                  UART0_IRQHandler
#define UART_0_INST_INT_IRQN                                      UART0_INT_IRQn
#define GPIO_UART_0_RX_PORT                                                GPIOA
#define GPIO_UART_0_TX_PORT                                                GPIOA
#define GPIO_UART_0_RX_PIN                                        DL_GPIO_PIN_11
#define GPIO_UART_0_TX_PIN                                        DL_GPIO_PIN_10
#define GPIO_UART_0_IOMUX_RX                                     (IOMUX_PINCM22)
#define GPIO_UART_0_IOMUX_TX                                     (IOMUX_PINCM21)
#define GPIO_UART_0_IOMUX_RX_FUNC                      IOMUX_PINCM22_PF_UART0_RX
#define GPIO_UART_0_IOMUX_TX_FUNC                      IOMUX_PINCM21_PF_UART0_TX
#define UART_0_BAUD_RATE                                                (115200)
#define UART_0_IBRD_4_MHZ_115200_BAUD                                        (2)
#define UART_0_FBRD_4_MHZ_115200_BAUD                                       (11)
/* Defines for UART_1 */
#define UART_1_INST                                                        UART1
#define UART_1_INST_FREQUENCY                                            4000000
#define UART_1_INST_IRQHandler                                  UART1_IRQHandler
#define UART_1_INST_INT_IRQN                                      UART1_INT_IRQn
#define GPIO_UART_1_RX_PORT                                                GPIOA
#define GPIO_UART_1_TX_PORT                                                GPIOA
#define GPIO_UART_1_RX_PIN                                         DL_GPIO_PIN_9
#define GPIO_UART_1_TX_PIN                                         DL_GPIO_PIN_8
#define GPIO_UART_1_IOMUX_RX                                     (IOMUX_PINCM20)
#define GPIO_UART_1_IOMUX_TX                                     (IOMUX_PINCM19)
#define GPIO_UART_1_IOMUX_RX_FUNC                      IOMUX_PINCM20_PF_UART1_RX
#define GPIO_UART_1_IOMUX_TX_FUNC                      IOMUX_PINCM19_PF_UART1_TX
#define UART_1_BAUD_RATE                                                (115200)
#define UART_1_IBRD_4_MHZ_115200_BAUD                                        (2)
#define UART_1_FBRD_4_MHZ_115200_BAUD                                       (11)
/* Defines for UART_2 */
#define UART_2_INST                                                        UART2
#define UART_2_INST_FREQUENCY                                           32000000
#define UART_2_INST_IRQHandler                                  UART2_IRQHandler
#define UART_2_INST_INT_IRQN                                      UART2_INT_IRQn
#define GPIO_UART_2_RX_PORT                                                GPIOB
#define GPIO_UART_2_TX_PORT                                                GPIOB
#define GPIO_UART_2_RX_PIN                                        DL_GPIO_PIN_16
#define GPIO_UART_2_TX_PIN                                        DL_GPIO_PIN_15
#define GPIO_UART_2_IOMUX_RX                                     (IOMUX_PINCM33)
#define GPIO_UART_2_IOMUX_TX                                     (IOMUX_PINCM32)
#define GPIO_UART_2_IOMUX_RX_FUNC                      IOMUX_PINCM33_PF_UART2_RX
#define GPIO_UART_2_IOMUX_TX_FUNC                      IOMUX_PINCM32_PF_UART2_TX
#define UART_2_BAUD_RATE                                                (115200)
#define UART_2_IBRD_32_MHZ_115200_BAUD                                      (17)
#define UART_2_FBRD_32_MHZ_115200_BAUD                                      (23)
/* Defines for UART_3 */
#define UART_3_INST                                                        UART3
#define UART_3_INST_FREQUENCY                                            4000000
#define UART_3_INST_IRQHandler                                  UART3_IRQHandler
#define UART_3_INST_INT_IRQN                                      UART3_INT_IRQn
#define GPIO_UART_3_RX_PORT                                                GPIOB
#define GPIO_UART_3_TX_PORT                                                GPIOB
#define GPIO_UART_3_RX_PIN                                        DL_GPIO_PIN_13
#define GPIO_UART_3_TX_PIN                                        DL_GPIO_PIN_12
#define GPIO_UART_3_IOMUX_RX                                     (IOMUX_PINCM30)
#define GPIO_UART_3_IOMUX_TX                                     (IOMUX_PINCM29)
#define GPIO_UART_3_IOMUX_RX_FUNC                      IOMUX_PINCM30_PF_UART3_RX
#define GPIO_UART_3_IOMUX_TX_FUNC                      IOMUX_PINCM29_PF_UART3_TX
#define UART_3_BAUD_RATE                                                  (9600)
#define UART_3_IBRD_4_MHZ_9600_BAUD                                         (26)
#define UART_3_FBRD_4_MHZ_9600_BAUD                                          (3)





/* Defines for ADC12_0 */
#define ADC12_0_INST                                                        ADC1
#define ADC12_0_INST_IRQHandler                                  ADC1_IRQHandler
#define ADC12_0_INST_INT_IRQN                                    (ADC1_INT_IRQn)
#define ADC12_0_ADCMEM_ch0                                    DL_ADC12_MEM_IDX_0
#define ADC12_0_ADCMEM_ch0_REF                   DL_ADC12_REFERENCE_VOLTAGE_VDDA
#define ADC12_0_ADCMEM_ch0_REF_VOLTAGE_V                                     3.3
#define GPIO_ADC12_0_C4_PORT                                               GPIOB
#define GPIO_ADC12_0_C4_PIN                                       DL_GPIO_PIN_17
#define GPIO_ADC12_0_IOMUX_C4                                    (IOMUX_PINCM43)
#define GPIO_ADC12_0_IOMUX_C4_FUNC                (IOMUX_PINCM43_PF_UNCONNECTED)



/* Defines for led_1: GPIOB.14 with pinCMx 31 on package pin 2 */
#define borad_led_1_PORT                                                 (GPIOB)
#define borad_led_1_PIN                                         (DL_GPIO_PIN_14)
#define borad_led_1_IOMUX                                        (IOMUX_PINCM31)
/* Defines for led_2: GPIOA.29 with pinCMx 4 on package pin 36 */
#define borad_led_2_PORT                                                 (GPIOA)
#define borad_led_2_PIN                                         (DL_GPIO_PIN_29)
#define borad_led_2_IOMUX                                         (IOMUX_PINCM4)
/* Defines for led_3: GPIOA.28 with pinCMx 3 on package pin 35 */
#define borad_led_3_PORT                                                 (GPIOA)
#define borad_led_3_PIN                                         (DL_GPIO_PIN_28)
#define borad_led_3_IOMUX                                         (IOMUX_PINCM3)
/* Defines for buzzer: GPIOA.14 with pinCMx 36 on package pin 7 */
#define borad_buzzer_PORT                                                (GPIOA)
#define borad_buzzer_PIN                                        (DL_GPIO_PIN_14)
#define borad_buzzer_IOMUX                                       (IOMUX_PINCM36)
/* Defines for key_1: GPIOB.21 with pinCMx 49 on package pin 20 */
#define borad_key_1_PORT                                                 (GPIOB)
#define borad_key_1_PIN                                         (DL_GPIO_PIN_21)
#define borad_key_1_IOMUX                                        (IOMUX_PINCM49)
/* Defines for key_3: GPIOA.26 with pinCMx 59 on package pin 30 */
#define borad_key_3_PORT                                                 (GPIOA)
#define borad_key_3_PIN                                         (DL_GPIO_PIN_26)
#define borad_key_3_IOMUX                                        (IOMUX_PINCM59)
/* Defines for key_4: GPIOB.23 with pinCMx 51 on package pin 22 */
#define borad_key_4_PORT                                                 (GPIOB)
#define borad_key_4_PIN                                         (DL_GPIO_PIN_23)
#define borad_key_4_IOMUX                                        (IOMUX_PINCM51)
/* Defines for key_2: GPIOA.27 with pinCMx 60 on package pin 31 */
#define borad_key_2_PORT                                                 (GPIOA)
#define borad_key_2_PIN                                         (DL_GPIO_PIN_27)
#define borad_key_2_IOMUX                                        (IOMUX_PINCM60)
/* Port definition for Pin Group oled */
#define oled_PORT                                                        (GPIOB)

/* Defines for CLK: GPIOB.9 with pinCMx 26 on package pin 61 */
#define oled_CLK_PIN                                             (DL_GPIO_PIN_9)
#define oled_CLK_IOMUX                                           (IOMUX_PINCM26)
/* Defines for MOSI: GPIOB.8 with pinCMx 25 on package pin 60 */
#define oled_MOSI_PIN                                            (DL_GPIO_PIN_8)
#define oled_MOSI_IOMUX                                          (IOMUX_PINCM25)
/* Defines for RES: GPIOB.10 with pinCMx 27 on package pin 62 */
#define oled_RES_PIN                                            (DL_GPIO_PIN_10)
#define oled_RES_IOMUX                                           (IOMUX_PINCM27)
/* Defines for DC: GPIOB.11 with pinCMx 28 on package pin 63 */
#define oled_DC_PIN                                             (DL_GPIO_PIN_11)
#define oled_DC_IOMUX                                            (IOMUX_PINCM28)
/* Port definition for Pin Group Gray_Address */
#define Gray_Address_PORT                                                (GPIOA)

/* Defines for PIN_0: GPIOA.1 with pinCMx 2 on package pin 34 */
#define Gray_Address_PIN_0_PIN                                   (DL_GPIO_PIN_1)
#define Gray_Address_PIN_0_IOMUX                                  (IOMUX_PINCM2)
/* Defines for PIN_1: GPIOA.0 with pinCMx 1 on package pin 33 */
#define Gray_Address_PIN_1_PIN                                   (DL_GPIO_PIN_0)
#define Gray_Address_PIN_1_IOMUX                                  (IOMUX_PINCM1)
/* Defines for PIN_2: GPIOA.31 with pinCMx 6 on package pin 39 */
#define Gray_Address_PIN_2_PIN                                  (DL_GPIO_PIN_31)
#define Gray_Address_PIN_2_IOMUX                                  (IOMUX_PINCM6)
/* Defines for AN1: GPIOA.18 with pinCMx 40 on package pin 11 */
#define TB6612_AN1_PORT                                                  (GPIOA)
#define TB6612_AN1_PIN                                          (DL_GPIO_PIN_18)
#define TB6612_AN1_IOMUX                                         (IOMUX_PINCM40)
/* Defines for AN2: GPIOA.15 with pinCMx 37 on package pin 8 */
#define TB6612_AN2_PORT                                                  (GPIOA)
#define TB6612_AN2_PIN                                          (DL_GPIO_PIN_15)
#define TB6612_AN2_IOMUX                                         (IOMUX_PINCM37)
/* Defines for BN1: GPIOA.22 with pinCMx 47 on package pin 18 */
#define TB6612_BN1_PORT                                                  (GPIOA)
#define TB6612_BN1_PIN                                          (DL_GPIO_PIN_22)
#define TB6612_BN1_IOMUX                                         (IOMUX_PINCM47)
/* Defines for BN2: GPIOB.22 with pinCMx 50 on package pin 21 */
#define TB6612_BN2_PORT                                                  (GPIOB)
#define TB6612_BN2_PIN                                          (DL_GPIO_PIN_22)
#define TB6612_BN2_IOMUX                                         (IOMUX_PINCM50)
/* Defines for left_A: GPIOB.27 with pinCMx 58 on package pin 29 */
#define ENCODER_left_A_PORT                                              (GPIOB)
// pins affected by this interrupt request:["left_A","left_B"]
#define ENCODER_GPIOB_INT_IRQN                                  (GPIOB_INT_IRQn)
#define ENCODER_GPIOB_INT_IIDX                  (DL_INTERRUPT_GROUP1_IIDX_GPIOB)
#define ENCODER_left_A_IIDX                                 (DL_GPIO_IIDX_DIO27)
#define ENCODER_left_A_PIN                                      (DL_GPIO_PIN_27)
#define ENCODER_left_A_IOMUX                                     (IOMUX_PINCM58)
/* Defines for left_B: GPIOB.26 with pinCMx 57 on package pin 28 */
#define ENCODER_left_B_PORT                                              (GPIOB)
#define ENCODER_left_B_IIDX                                 (DL_GPIO_IIDX_DIO26)
#define ENCODER_left_B_PIN                                      (DL_GPIO_PIN_26)
#define ENCODER_left_B_IOMUX                                     (IOMUX_PINCM57)
/* Defines for right_A: GPIOA.24 with pinCMx 54 on package pin 25 */
#define ENCODER_right_A_PORT                                             (GPIOA)
// pins affected by this interrupt request:["right_A","right_B"]
#define ENCODER_GPIOA_INT_IRQN                                  (GPIOA_INT_IRQn)
#define ENCODER_GPIOA_INT_IIDX                  (DL_INTERRUPT_GROUP1_IIDX_GPIOA)
#define ENCODER_right_A_IIDX                                (DL_GPIO_IIDX_DIO24)
#define ENCODER_right_A_PIN                                     (DL_GPIO_PIN_24)
#define ENCODER_right_A_IOMUX                                    (IOMUX_PINCM54)
/* Defines for right_B: GPIOA.25 with pinCMx 55 on package pin 26 */
#define ENCODER_right_B_PORT                                             (GPIOA)
#define ENCODER_right_B_IIDX                                (DL_GPIO_IIDX_DIO25)
#define ENCODER_right_B_PIN                                     (DL_GPIO_PIN_25)
#define ENCODER_right_B_IOMUX                                    (IOMUX_PINCM55)


/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_motor_init(void);
void SYSCFG_DL_OLED_refresh_init(void);
void SYSCFG_DL_encoder_init(void);
void SYSCFG_DL_UART_0_init(void);
void SYSCFG_DL_UART_1_init(void);
void SYSCFG_DL_UART_2_init(void);
void SYSCFG_DL_UART_3_init(void);
void SYSCFG_DL_ADC12_0_init(void);


bool SYSCFG_DL_saveConfiguration(void);
bool SYSCFG_DL_restoreConfiguration(void);

#ifdef __cplusplus
}
#endif

#endif /* ti_msp_dl_config_h */
