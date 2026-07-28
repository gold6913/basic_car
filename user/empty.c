/**
 * @file    empty.c
 * @brief   主程序入口，包含菜单选择、加载过渡与任务调度逻辑
 * @note    上电后默认进入菜单模式（menu_start=0），通过按键确认后依次经过
 *          加载过渡（menu_start=1）→ 运行模式（menu_start=2）
 */

/*========================== 头文件包含 ==========================*/

#include "ADC.h"                                 /* ADC 模数转换驱动            */
#include "No_Mcu_Ganv_Grayscale_Sensor_Config.h" /* 灰度传感器配置               */
#include "borad.h"                               /* 板级外设初始化               */
#include "delay.h"                               /* 延时函数                    */
#include "encoder.h"                             /* 编码器测速                  */
#include "jy61p.h"                               /* JY61P 陀螺仪/姿态传感器       */
#include "menu.h"                                /* 菜单显示与交互               */
#include "oled.h"                                /* OLED 显示屏驱动              */
#include "pid.h"                                 /* PID 控制算法                */
#include "start.h"                               /* 启动/任务调度                */
#include "tb6612.h"                              /* TB6612 双路电机驱动          */
#include "ti_msp_dl_config.h"                    /* TI MSPM0 驱动库配置          */
#include "uart.h"                                /* 串口通信                    */
#include "zdt_motor.h"                           /* 电机驱动                    */
/*========================== 全局变量定义 ==========================*/

/*========================== 主函数 ==========================*/

/**
  * @brief    主函数，系统初始化后进入菜单/运行双模式循环
  * @param    无
  * @retval   无
  */
int main(void)
{
    init(); /* 系统初始化 */
    while (1)
    {
        printf("%d,\r\n",menu_cursor);
        /*--------------- 菜单模式 ---------------*/
        if (menu_start == MODE_MENU)
        {
            menu_render();      /* 渲染主菜单页面 */
            menu_handle_keys(); /* 按键处理（光标移动 / 确认进入运行） */
        }
        /*--------------- 加载过渡 ---------------*/
        else if (menu_start == MODE_LOADING)
        {
            loading_show();     /* 显示加载画面，延时后 menu_start → 2 */
        }
        /*--------------- 运行模式 ---------------*/
        else
        {
            Collect_Data();     /* 采集传感器数据 */
            menu_render();      /* 渲染当前页面（传感器 / 电机转速） */
            menu_handle_keys(); /* 按键处理（页面切换） */
            target_start();     /* 执行目标任务调度 */
        }
        oled_updat();
    }
}
