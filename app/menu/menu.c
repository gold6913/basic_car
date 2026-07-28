/**
 * @file    menu.c
 * @brief   OLED 多页面菜单系统（枚举 + 结构体注册表模式）
 * @note    所有页面通过 menuPages[] 静态注册，运行时仅维护
 *          menu_currentPage / menu_cursor / menu_start 三个变量
 */

#include "menu.h"
#include "encoder.h"
#include "start.h"


/*========================== 运行时状态 ==========================*/

PageId menu_currentPage = PAGE_MAIN; /**< 当前活跃页面                  */
uint8_t menu_cursor = 0;              /**< 当前光标位置（仅 maxItems>0 有效） */
uint8_t menu_start = 0;              /**< 模式: 0=菜单界面, 1=运行模式 */
uint8_t refresh = 0;                 /**< OLED 刷新标志                 */

/*========================== 菜单初始化 ==========================*/

/**
 * @brief    菜单初始化，重置到主菜单页面
 * @param    无
 * @retval   无
 */
void menu_init(void)
{
    menu_currentPage = PAGE_MAIN;
    menu_cursor = 0;
}

/*========================== 页面绘制函数（static） ==========================*/

/**
 * @brief    绘制主菜单页面（4 个 target + 光标）
 * @param    cursor  ：当前光标位置（0 ~ 3）
 * @retval   无
 */
static void drawMain(uint8_t cursor)
{
    OLED_DrawFill(0, cursor * 16 - 15, 8, 16, 0);
    OLED_DrawFill(0, cursor * 16 + 49, 8, 16, 0);
    OLED_ShowString(8, 0, " target1", 16, 1);
    OLED_ShowString(8, 16, " target2", 16, 1);
    OLED_ShowString(8, 32, " target3", 16, 1);
    OLED_ShowString(8, 48, " target4", 16, 1);
    OLED_ShowString(0, cursor * 16 + 1, ">", 16, 1);
}

/**
 * @brief    绘制传感器数据页面（偏航角 / 灰度偏差 / 8 通道灰度）
 * @param    cursor  ：未使用
 * @retval   无
 */
static void drawSensor(uint8_t cursor)
{
    (void)cursor; /* 本页无光标 */

    /* 第 1 行：偏航角 */
    OLED_ShowString(0, 0, "Yaw:", 16, 1);
    OLED_ShowSFNum(32, 0, measure_yaw, 3, 16, 1);

    /* 第 2 行：灰度偏差 */
    OLED_ShowString(0, 16, "Err:", 16, 1);
    OLED_ShowSNum(32, 16, measure_gray, 3, 16, 1);

    /* 第 3 行：8 通道灰度 */
    OLED_ShowString(0, 32, "Gray:", 16, 1);
    for (int i = 0; i < 8; i++)
    {
        OLED_ShowNum(40 + i * 10, 32, gray[i], 1, 16, 1);
    }

    /* 第 4 行：页面指示 */
    OLED_ShowString(112, 48, "P0", 16, 1);
}

/**
 * @brief    绘制电机转速页面
 * @param    cursor  ：未使用
 * @retval   无
 */
static void drawMotor(uint8_t cursor)
{
    (void)cursor; /* 本页无光标 */

    /* 第 1 行：左轮转速 */
    OLED_ShowString(0, 0, "L_Speed:", 16, 1);
    OLED_ShowSNum(64, 0, encoder_left_speed, 4, 16, 1);

    /* 第 2 行：右轮转速 */
    OLED_ShowString(0, 16, "R_Speed:", 16, 1);
    OLED_ShowSNum(64, 16, encoder_right_speed, 4, 16, 1);

    /* 第 3 行：占位（可扩展） */

    /* 第 4 行：页面指示 */
    OLED_ShowString(112, 48, "P1", 16, 1);
}

/*========================== 页面按键回调（static） ==========================*/

/**
 * @brief    主菜单按键回调
 * @note     按键2: 光标下移（0→1→2→3→0 循环）
 *           按键3: 确认选择，进入运行模式并切换到传感器页
 * @param    key  ：按键编号
 * @retval   无
 */
static void onKeyMain(uint8_t key)
{
    if (key == 2)
    {
        /* 光标循环：0 → 1 → 2 → 3 → 0 */
        menu_cursor = (menu_cursor + 1) % 4;
    }
    else if (key == 3)
    {
        /* 确认：进入运行模式 */
        menu_start = 1;
        OLED_Clear();
        menu_switchTo(PAGE_SENSOR);
    }
}

/**
 * @brief    传感器页按键回调
 * @note     按键4: 切换到电机转速页
 * @param    key  ：按键编号
 * @retval   无
 */
static void onKeySensor(uint8_t key)
{
    if (key == 4)
    {
        menu_switchTo(PAGE_MOTOR);
    }
}

/**
 * @brief    电机转速页按键回调
 * @note     按键4: 切换回传感器页
 * @param    key  ：按键编号
 * @retval   无
 */
static void onKeyMotor(uint8_t key)
{
    if (key == 4)
    {
        menu_switchTo(PAGE_SENSOR);
    }
}

/*========================== 页面注册表（const 放 Flash） ==========================*/

/**
 * @brief  页面注册表，通过 PageId 索引即可获取对应页面的所有信息
 */
static const MenuPage menuPages[PAGE_COUNT] = {
    [PAGE_MAIN] = {PAGE_MAIN, "Main", 4, drawMain, NULL, onKeyMain},
    [PAGE_SENSOR] = {PAGE_SENSOR, "Sensor", 0, drawSensor, NULL, onKeySensor},
    [PAGE_MOTOR] = {PAGE_MOTOR, "Motor", 0, drawMotor, NULL, onKeyMotor},
};

/*========================== 统一调度接口 ==========================*/

/**
 * @brief    渲染当前页面
 * @note     通过 menuPages[currentPage].draw 分发
 * @param    无
 * @retval   无
 */
void menu_render(void)
{
    if (menu_currentPage < PAGE_COUNT && menuPages[menu_currentPage].draw)
    {
        menuPages[menu_currentPage].draw(menu_cursor);
    }
}

/**
 * @brief    按键分发处理（统一入口）
 * @note     先等待按键释放，再调用当前页面的 onKey 回调；
 *           同时根据 menu_start 决定按键来源（菜单模式 vs 运行模式）
 * @param    无
 * @retval   无
 */
void menu_handle_keys(void)
{ 
    uint8_t key = 0;

    /* 扫描按键 2/3/4，检测按下 */
    if (key_get(2) == 0)
    {
        key = 2;
    }
    else if (key_get(3) == 0)
    {
        key = 3;
    }
    else if (key_get(4) == 0)
    {
        key = 4;
    }

    if (key == 0)
        return; /* 无按键按下 */

    while (key_get(key) == 0)
    {
    } /* 等待按键释放 */

    /* 分发到当前页面的 onKey 回调 */
    if (menu_currentPage < PAGE_COUNT && menuPages[menu_currentPage].onKey)
    {
        menuPages[menu_currentPage].onKey(key);
    }
}

/**
 * @brief    切换到指定页面
 * @param    page  ：目标页面 ID
 * @retval   无
 */
void menu_switchTo(PageId page)
{
    if (page >= PAGE_COUNT)
        return;

    menu_currentPage = page;
    //menu_cursor = 0; /* 切换页面时重置光标 */

    OLED_Clear(); /* 清屏，避免上一页残留 */

    if (menuPages[page].onEnter)
    {
        menuPages[page].onEnter(); /* 执行进入回调 */
    }
}

/*========================== OLED 刷新 ==========================*/

/**
 * @brief    OLED 刷新（主循环中调用）
 * @param    无
 * @retval   无
 */
void oled_updat(void)
{
    if (refresh == 1)
    {
        refresh = 0;    /* 清除刷新标志 */
        OLED_Refresh(); /* 刷新 OLED 显示 */
    }
}

/**
 * @brief    OLED 刷新定时器中断服务函数
 * @param    无
 * @retval   无
 */
void OLED_refresh_INST_IRQHandler(void)
{
    /* 清除定时器中断标志，防止重复触发 */
    DL_TimerA_clearInterruptStatus(OLED_refresh_INST, DL_TIMERA_INTERRUPT_ZERO_EVENT);
    refresh = 1; /* 设置刷新标志，主循环中处理 OLED 刷新 */
}