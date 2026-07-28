/**
 * @file    menu.h
 * @brief   OLED 多页面菜单系统（枚举 + 结构体注册表模式）
 * @note    使用 PageId 枚举和 MenuPage 结构体统一管理所有页面，
 *          运行时仅维护 currentPage / cursor / menu_start 三个变量
 */

#ifndef __MENU_H__
#define __MENU_H__

/*========================== 头文件包含 ==========================*/

#include "ti_msp_dl_config.h"
#include "oled.h"
#include "borad.h"
#include <stdint.h>
#include "jy61p.h"

/*========================== 页面枚举 ==========================*/

/** 页面 ID 枚举，PAGE_COUNT 自动统计页面总数 */
typedef enum {
    PAGE_MAIN    = 0,  /**< 主菜单（target 选择）       */
    PAGE_SENSOR  = 1,  /**< 传感器数据（偏航角 / 灰度） */
    PAGE_MOTOR   = 2,  /**< 电机转速                   */
    PAGE_COUNT         /**< 页面总数（自动）            */
} PageId;

/*========================== 页面结构体 ==========================*/

/**
  * @brief  菜单页面描述结构体
  * @note   id       ：页面唯一标识
  *         name     ：页面名称（调试用）
  *         maxItems ：光标可选项数量（0 表示无光标）
  *         draw     ：绘制函数，cursor 为当前光标位置
  *         onEnter  ：进入页面时的回调（可为 NULL）
  *         onKey    ：按键回调，参数为按键编号（可为 NULL）
  */
typedef struct {
    PageId      id;                          /**< 页面 ID                     */
    const char *name;                        /**< 页面名称                    */
    uint8_t     maxItems;                    /**< 光标项数（0=无光标）        */
    void      (*draw)(uint8_t cursor);       /**< 绘制回调                    */
    void      (*onEnter)(void);              /**< 进入回调                    */
    void      (*onKey)(uint8_t key);         /**< 按键回调                    */
} MenuPage;

/*========================== 全局变量 (extern) ==========================*/

extern PageId  menu_currentPage;  /**< 当前活跃页面                   */
extern uint8_t menu_cursor;       /**< 当前光标位置（仅 maxItems>0 有效） */
extern uint8_t menu_start;        /**< 模式标志: 0=菜单界面, 1=运行模式  */
extern uint8_t refresh;           /**< OLED 刷新标志                   */

/*========================== 函数声明 ==========================*/

void menu_init(void);                      /**< 菜单初始化（设置默认页）      */
void menu_render(void);                    /**< 渲染当前页面                  */
void menu_handle_keys(void);               /**< 按键分发处理                  */
void menu_switchTo(PageId page);           /**< 切换到指定页面                */
void oled_updat(void);                     /**< OLED 刷新                    */

#endif /* __MENU_H__ */
