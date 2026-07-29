/**
 * @file    menu.h
 * @brief   OLED 多页面菜单系统
 */

#ifndef __MENU_H__
#define __MENU_H__

#include "ti_msp_dl_config.h"
#include "oled.h"
#include "borad.h"
#include <stdint.h>
#include "jy61p.h"

/** 按键编号 */
#define KEY_DOWN    2
#define KEY_CONFIRM 3
#define KEY_SWITCH  4

/** 系统模式 */
#define MODE_MENU    0
#define MODE_LOADING 1
#define MODE_RUN     2

/** 页面 ID */
typedef enum {
    PAGE_MAIN   = 0,
    PAGE_SENSOR = 1,
    PAGE_MOTOR  = 2,
    PAGE_COUNT
} PageId;

extern PageId  menu_currentPage;
extern uint8_t menu_cursor;
extern uint8_t menu_start;
extern uint8_t refresh;
extern uint32_t car_runtime_sec;

void menu_init(void);
void menu_render(void);
void menu_handle_keys(void);
void menu_switchTo(PageId page);
void oled_updat(void);

#endif
