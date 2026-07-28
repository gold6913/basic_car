/**
 * @file    menu.c
 * @brief   OLED 多页面菜单系统
 * @note    通过 PageId + switch-case 直接分发，不引入结构体注册表
 */

#include "menu.h"
#include "encoder.h"
#include "start.h"

/*========================== 运行时状态 ==========================*/

PageId  menu_currentPage = PAGE_MAIN;
uint8_t menu_cursor      = 0;
uint8_t menu_start       = 0;
uint8_t refresh          = 0;

/*========================== 页面绘制 ==========================*/

/**
 * @brief  绘制主菜单页面
 * @param  cursor  当前光标位置（0 ~ 3）
 */
static void drawMain(uint8_t cursor)
{
    OLED_DrawFill(0, 0, 8, 64, 0);
    OLED_ShowString(8, 0,  (u8 *)" target1", 16, 1);
    OLED_ShowString(8, 16, (u8 *)" target2", 16, 1);
    OLED_ShowString(8, 32, (u8 *)" target3", 16, 1);
    OLED_ShowString(8, 48, (u8 *)" target4", 16, 1);
    OLED_ShowString(0, cursor * 16 + 1, (u8 *)">", 16, 1);
}

/**
 * @brief  绘制传感器数据页面
 */
static void drawSensor(void)
{
    OLED_ShowString(0, 0, (u8 *)"Yaw:", 16, 1);
    OLED_ShowSFNum(32, 0, measure_yaw, 3, 16, 1);

    OLED_ShowString(0, 16, (u8 *)"Err:", 16, 1);
    OLED_ShowSNum(32, 16, measure_gray, 3, 16, 1);

    OLED_ShowString(0, 32, (u8 *)"Gray:", 16, 1);
    for (int i = 0; i < 8; i++)
    {
        OLED_ShowNum(40 + i * 10, 32, gray[i], 1, 16, 1);
    }

    OLED_ShowString(112, 48, (u8 *)"P0", 16, 1);
}

/**
 * @brief  绘制电机转速页面
 */
static void drawMotor(void)
{
    OLED_ShowString(0, 0, (u8 *)"L_Speed:", 16, 1);
    OLED_ShowSNum(64, 0, encoder_left_speed, 4, 16, 1);

    OLED_ShowString(0, 16, (u8 *)"R_Speed:", 16, 1);
    OLED_ShowSNum(64, 16, encoder_right_speed, 4, 16, 1);

    OLED_ShowString(112, 48, (u8 *)"P1", 16, 1);
}

/*========================== 按键处理 ==========================*/

/**
 * @brief  主菜单按键处理
 * @note   KEY_DOWN: 光标下移循环  KEY_CONFIRM: 确认并切换到传感器页
 */
static void onKeyMain(uint8_t key)
{
    if (key == KEY_DOWN)
    {
        menu_cursor = (menu_cursor + 1) % 4;
    }
    else if (key == KEY_CONFIRM)
    {
        menu_start = MODE_LOADING;
        OLED_Clear();
        menu_switchTo(PAGE_SENSOR);
    }
}

/**
 * @brief  传感器页按键处理
 * @note   KEY_SWITCH: 切换到电机转速页
 */
static void onKeySensor(uint8_t key)
{
    if (key == KEY_SWITCH)
    {
        menu_switchTo(PAGE_MOTOR);
    }
}

/**
 * @brief  电机转速页按键处理
 * @note   KEY_SWITCH: 切换回传感器页
 */
static void onKeyMotor(uint8_t key)
{
    if (key == KEY_SWITCH)
    {
        menu_switchTo(PAGE_SENSOR);
    }
}

/*========================== 调度入口 ==========================*/

/**
 * @brief  菜单初始化
 */
void menu_init(void)
{
    menu_currentPage = PAGE_MAIN;
    menu_cursor = 0;
}

/**
 * @brief  渲染当前页面
 */
void menu_render(void)
{
    switch (menu_currentPage)
    {
    case PAGE_MAIN:
        drawMain(menu_cursor);
        break;
    case PAGE_SENSOR:
        drawSensor();
        break;
    case PAGE_MOTOR:
        drawMotor();
        break;
    default:
        break;
    }
}

/**
 * @brief  按键扫描与分发
 * @note   扫描 KEY_DOWN / KEY_CONFIRM / KEY_SWITCH，等待释放后分发给当前页面
 */
void menu_handle_keys(void)
{
    uint8_t key = 0;

    if      (key_get(KEY_DOWN)    == 0) { key = KEY_DOWN;    }
    else if (key_get(KEY_CONFIRM) == 0) { key = KEY_CONFIRM; }
    else if (key_get(KEY_SWITCH)  == 0) { key = KEY_SWITCH;  }

    if (key == 0) return;

    while (key_get(key) == 0) {}  /* 等待按键释放 */

    switch (menu_currentPage)
    {
    case PAGE_MAIN:   onKeyMain(key);   break;
    case PAGE_SENSOR: onKeySensor(key); break;
    case PAGE_MOTOR:  onKeyMotor(key);  break;
    default: break;
    }
}

/**
 * @brief  切换到指定页面
 * @param  page  目标页面 ID
 */
void menu_switchTo(PageId page)
{
    if (page >= PAGE_COUNT) return;

    menu_currentPage = page;
    menu_cursor = 0;
    OLED_Clear();
}

/*========================== OLED 刷新 ==========================*/

/**
 * @brief  OLED 刷新（主循环中调用）
 */
void oled_updat(void)
{
    if (refresh == 1)
    {
        refresh = 0;
        OLED_Refresh();
    }
}

/**
 * @brief  OLED 刷新定时器中断
 */
void OLED_refresh_INST_IRQHandler(void)
{
    DL_TimerA_clearInterruptStatus(OLED_refresh_INST, DL_TIMERA_INTERRUPT_ZERO_EVENT);
    refresh = 1;
}