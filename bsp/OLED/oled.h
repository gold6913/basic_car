/**
 * @file oled.h
 * @author maker114
 * @brief OLED库
 * @version 2.2
 * @date 2024-07-13
 */

/*
图形库组成：
|-OLED 基础图形库
    |-oled.h
    |-oledfont.h <-存储字体
    |-bmp.h <-存储图片数组
    |-oled.c
|-MENU 各式菜单(已禁用)
    |-oled_menu.h
    |-oled_menu.c
|-START 启动配置与底层实现
    |-oled_start.h
    |-oled_start.c
*/

#ifndef __OLED_H
#define __OLED_H


#include "stdint.h"
#include "stdlib.h"
#include <math.h>
#include "ti_msp_dl_config.h"

//数据类型宏定义
typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;


#define OLED_SCL_Clr() DL_GPIO_clearPins(oled_PORT ,oled_CLK_PIN)// SCL
#define OLED_SCL_Set() DL_GPIO_setPins(oled_PORT ,oled_CLK_PIN)

#define OLED_SDA_Clr() DL_GPIO_clearPins(oled_PORT,oled_MOSI_PIN)// SDA
#define OLED_SDA_Set() DL_GPIO_setPins(oled_PORT ,oled_MOSI_PIN)

#define OLED_RES_Clr()DL_GPIO_clearPins(oled_PORT,oled_RES_PIN) // RES
#define OLED_RES_Set()DL_GPIO_setPins(oled_PORT ,oled_RES_PIN)

#define OLED_DC_Clr() DL_GPIO_clearPins(oled_PORT,oled_DC_PIN)// DC
#define OLED_DC_Set() DL_GPIO_setPins(oled_PORT ,oled_DC_PIN )

#define OLED_CS_Clr() 
#define OLED_CS_Set() 
#define OLED_CMD 0
#define OLED_DATA 1


// 屏幕中心坐标
#define Middle_X 64
#define Middle_Y 32
// 部分数学实现
#define RADIAN(angle) ((angle == 0) ? 0 : (3.14159f * angle / 180))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define SWAP(x, y)   \
    (y) = (x) + (y); \
    (x) = (y) - (x); \
    (y) = (y) - (x);

/*========================OLED========================*/

// |--SHOW
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size1, u8 mode); // 显示单个字符
void OLED_ShowString(u8 x, u8 y, u8 *chr, u8 size1, u8 mode);             // 显示字符void OLED_ShowNum(u8 x, u8 y, int num, u8 len, u8 size1, u8 mode);        // 显示数字
void OLED_ShowSNum(u8 x, u8 y, int num, u8 len, u8 size1, u8 mode);       // 显示有符号数字
void OLED_ShowChinese(u8 x, u8 y, u8 num, u8 size1, u8 mode);             // 显示中文字符
void OLED_ShowPicture(u8 x, u8 y, u8 sizex, u8 sizey, u8 BMP[], u8 mode); // 显示图片
void OLED_ShowNum16(u8 x, u8 y, u8 num, u8 size, u8 mode);                // 显示16进制数字
void OLED_ShowFNum(u8 x, u8 y, float data, u8 len, u8 size, u8 mode);     // 显示浮点数
void OLED_ShowSFNum(u8 x, u8 y, float data, u8 len, u8 size, u8 mode);    // 显示带有符号的浮点数
void OLED_ShowNum(u8 x, u8 y, int num, u8 len, u8 size1, u8 mode);

// |--DRAW
void OLED_ClearPoint(u8 x, u8 y);                                          // 清除OLED上的某个点
void OLED_DrawPoint(u8 x, u8 y, u8 t);                                     // 在OLED上画一个点
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2, u8 mode);                   // 在OLED上画一条线
void OLED_DrawCircle_math(u8 x, u8 y, u8 r);                               // 在OLED上绘制圆形（使用数学方法）
void OLED_DrawFastHLine(int x, int y, int w, int mode);                    // 快速画水平线
void OLED_DrawFastVLine(int x, int y, int h, int mode);                    // 快速画垂直线
void OLED_DrawCircleQuadrant(u8 x, u8 y, u8 r, u8 quadrant, u8 mode);      // 绘制圆形的一个象限
void OLED_DrawFill(int x, int y, int w, int h, int mode);                  // 填充矩形区域
void OLED_DrawFill_T(int x, int y, int w, int h, u8 Thickness, int mode);  // 以特定厚度填充矩形区域
void OLED_DrawBox(int x, int y, int w, int h, int mode);                   // 绘制矩形框
void OLED_DrawRoundedBox(int x, int y, int w, int h, int r, int mode);     // 绘制圆角矩形框
void OLED_DrawDashedLine(u8 x1, u8 y1, u8 x2, u8 y2, u8 mode);             // 在OLED上绘制虚线
void OLED_DrawCircle(int usX_Center, int usY_Center, int usRadius);        // 在OLED上绘制圆形
void OLED_DrawParallelHLine(u8 x, u8 y, u8 w, u8 h, u8 section, u8 mode);  // 等间距画水平线
void OLED_DrawParallelVLine(u8 x, u8 y, u8 w, u8 h, u8 section, u8 mode);  // 等间距画垂直线
void OLED_DrawVacuumization(u8 x, u8 y, u8 w, u8 h);                       // 绘制"虚化"效果
void OLED_DrawBezierCurve(int x0, int y0, int x1, int y1, int x2, int y2); // 绘制贝塞尔曲线
void OLED_DrawBezierBox(int x, int y, int w, int h, int r);                // 绘制贝塞尔矩形框

/*========================OLED_MENU========================*///出于MSPM0的性能考虑，已将oled_menu部分全部移除，反正加上也用不了XD
//// |--CHANGE
//void OLED_MoveCursor(float goal_cursor, float *now_cursor, u8 delay);              // 移动光标
//void OLED_Rotate(u8 *x, u8 *y, u8 Core_x, u8 Core_y, float angle, float distance); // 输出旋转坐标
//void MENU_SetMode(int set, int Mode);                                              // 设置显示模式
//void MENU_RollingClear(void);                                                      // 清除滚动显示缓冲区
//void MENU_SignKeyFun(int put_in_fun(void));                                        // 菜单注册按键功能

//// |--DISPLAY
//void OLED_DrawCurve(float zoom, float skew, float data);              // 绘制曲线
//int MENU_NormalDisplay(u8 *menu[], u8 arr_len);                       // 正常显示菜单
//int MENU_SideDisplay(u8 *menu[], u8 arr_len, u8 w);                   // 侧边显示菜单
//void MENU_MiddleDisplay(u8 *chr, float MIN, float MAX, float *value); // 百分比菜单
////void MENU_USARTDisplay(void);                                         // USART显示（MSPM0暂不可用）
//void MENU_RollingDisplaced(u8 *chr);                                  // 滚动显示文字
//float MENU_ChangeDisplay(u8 *chr, float step, float num);             // 数值调节菜单

//// |--ANIMATIONS
//void Normal_First_IN(u8 *menu[], u8 arr_len, u8 size);      // 正常显示菜单的初次进入
//void Side_First_IN(u8 *menu[], u8 arr_len, u8 size, u8 w);  // 侧边显示菜单的初次进入
//void Side_First_OUT(u8 *menu[], u8 arr_len, u8 size, u8 w); // 侧边显示菜单的退出
//void Middle_First_IN(float data, float percent, u8 *chr);   // 中间显示功能的初次进入
//void Middle_First_OUT(float data, float percent, u8 *chr);  // 中间显示功能的退出
//void Change_First_IN(float num, u8 *chr);                   // 数值改变的初次进入
//void Change_First_OUT(float num, u8 *chr);                  // 数值改变的退出
//void Curve_First_IN(void);                                  // 曲线显示的初次进入

/*========================OLED_START========================*/

void OLED_ColorTurn(u8 i);                          // 颜色翻转
void OLED_DisplayTurn(u8 i);                        // 显示翻转
void OLED_WR_Byte(u8 dat, u8 mode);                 // 写一个字节数据或命令到OLED
void OLED_DisPlay_On(void);                         // 打开OLED显示
void OLED_DisPlay_Off(void);                        // 关闭OLED显示
void OLED_Refresh(void);                            // 刷新OLED显示内容
void OLED_Clear(void);                              // 清屏
void OLED_Init(void);                               // OLED初始化
void OELD_ChangeLuminance(u8 level);                // 调整亮度
void OLED_BufferSave(void);                         // 保存当前显示缓冲区
void OLED_BufferRegain(void);                       // 恢复保存的显示缓冲区
void OLED_ScrollDisplay(u8 num, u8 space, u8 mode); // OLED滚动显示

/*========================solf_IIC========================*/

void OLED_WR_Byte(u8 dat,u8 mode);
void Send_Byte(u8 dat);
void I2C_WaitAck(void);
void I2C_Stop(void);
void I2C_Start(void);
void IIC_delay(void);

#endif
