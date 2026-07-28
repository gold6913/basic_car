/**
 * @file oled_start.c
 * @author maker114
 * @brief OLED配置文件与底层实现
 * @version 2.2
 * @date 2024-02-12
 */
#include "oled.h"
#include "string.h"
u8 OLED_GRAM[128][8]; ///< OLED缓存区
unsigned char rx_buff[256]={0};
/**
 * ****************************************************************************************
 * @brief 反显函数
 * @param i 0:正常显示 1:反向显示
 * ****************************************************************************************
 */
// 反显函数
void OLED_ColorTurn(u8 i)
{
	if (i == 0)
	{
		OLED_WR_Byte(0xA6, OLED_CMD); // 正常显示
	}
	if (i == 1)
	{
		OLED_WR_Byte(0xA7, OLED_CMD); // 反色显示
	}
}

/**
 * ****************************************************************************************
 * @brief  屏幕旋转180度
 * @param i 0:正常显示 1:反转显示
 * ****************************************************************************************
 */
// 屏幕旋转180度
void OLED_DisplayTurn(u8 i)
{
	if (i == 0)
	{
		OLED_WR_Byte(0xC8, OLED_CMD); // 正常显示
		OLED_WR_Byte(0xA1, OLED_CMD);
	}
	if (i == 1)
	{
		OLED_WR_Byte(0xC0, OLED_CMD); // 反转显示
		OLED_WR_Byte(0xA0, OLED_CMD);
	}
}

/**
 * ****************************************************************************************
 * @brief 书写一个字节
 * @param dat 数据
 * @param cmd 指令
 * ****************************************************************************************
 */
void OLED_WR_Byte(u8 dat, u8 cmd)
{
	u8 i;
	if (cmd)
		OLED_DC_Set();
	else
		OLED_DC_Clr();
	OLED_CS_Clr();
	for (i = 0; i < 8; i++)
	{
		OLED_SCL_Clr();
		if (dat & 0x80)
			OLED_SDA_Set();
		else
			OLED_SDA_Clr();
		OLED_SCL_Set();
		dat <<= 1;
	}
	OLED_CS_Set();
	OLED_DC_Set();
}

/**
 * ****************************************************************************************
 * @brief 开启OLED显示
 * ****************************************************************************************
 */
// 开启OLED显示
void OLED_DisPlay_On(void)
{
	OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵使能
	OLED_WR_Byte(0x14, OLED_CMD); // 开启电荷泵
	OLED_WR_Byte(0xAF, OLED_CMD); // 点亮屏幕
}

/**
 * ****************************************************************************************
 * @brief 关闭OLED显示
 * ****************************************************************************************
 */
// 关闭OLED显示
void OLED_DisPlay_Off(void)
{
	OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵使能
	OLED_WR_Byte(0x10, OLED_CMD); // 关闭电荷泵
	OLED_WR_Byte(0xAE, OLED_CMD); // 关闭屏幕
}

/**
 * ****************************************************************************************
 * @brief 发送缓冲区
 * @note
 * - 使用1.3寸OLED显示屏时低列起始地址为0x02
 * - 使用0.96寸OLED显示屏时低起始地址为0x00
 * ****************************************************************************************
 */
// 更新显存到OLED
void OLED_Refresh(void)
{
	u8 i, n;
	for (i = 0; i < 8; i++)
	{
		OLED_WR_Byte(0xb0 + i, OLED_CMD); // 设置行起始地址
		OLED_WR_Byte(0x02, OLED_CMD);	  // 设置低列起始地址
		OLED_WR_Byte(0x10, OLED_CMD);	  // 设置高列起始地址
		for (n = 0; n < 128; n++)
			OLED_WR_Byte(OLED_GRAM[n][i], OLED_DATA);
	}
}
/**
 * ****************************************************************************************
 * @brief 清屏函数
 * @attention 取消了默认的清屏后发送缓冲区，若需要将屏幕内容全部清除需要在此函数后加上OLED_Refresh();
 * ****************************************************************************************
 */
// 清屏函数
void OLED_Clear(void)
{
	u8 i, n;
	for (i = 0; i < 8; i++)
	{
		for (n = 0; n < 128; n++)
		{
			OLED_GRAM[n][i] = 0; // 清除所有数据
		}
	}
	// OLED_Refresh(); // 更新显示  <-真碍事
}

/**
 * ****************************************************************************************
 * @brief 画点
 * @param x 0~127
 * @param y 0~63
 * @param t 1 填充 0,清空
 * ****************************************************************************************
 */
void OLED_DrawPoint(u8 x, u8 y, u8 t)
{

	if (x > 128 || y > 63)
	{
		return; // 超出范围了
	}

	u8 i, m, n;
	i = y / 8;
	m = y % 8;
	n = 1 << m;

	if (t)
	{
		OLED_GRAM[x][i] |= n;
	}
	else
	{
		OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
		OLED_GRAM[x][i] |= n;
		OLED_GRAM[x][i] = ~OLED_GRAM[x][i];
	}
}

/**
 * ****************************************************************************************
 * @brief OLED初始化
 * @note 注意初始化完成清屏后需要发送一次缓冲区
 * ****************************************************************************************
 */
void OLED_Init(void)
{

	OLED_RES_Clr();
	delay_cycles(32000000/1000*200);
	OLED_RES_Set();

	OLED_WR_Byte(0xAE, OLED_CMD); //--关闭OLED显示屏
	OLED_WR_Byte(0x00, OLED_CMD); //---设置低列地址
	OLED_WR_Byte(0x10, OLED_CMD); //---设置高列地址
	OLED_WR_Byte(0x40, OLED_CMD); //--设置起始行地址  设置映射RAM显示起始行（0x00~0x3F）
	OLED_WR_Byte(0x81, OLED_CMD); //--设置对比度控制寄存器
	OLED_WR_Byte(0xCF, OLED_CMD); // 设置SEG输出电流亮度
	OLED_WR_Byte(0xA1, OLED_CMD); //--设置SEG/列映射     0xa0左右反置 0xa1正常
	OLED_WR_Byte(0xC8, OLED_CMD); // 设置COM/行扫描方向   0xc0上下反置 0xc8正常
	OLED_WR_Byte(0xA6, OLED_CMD); //--设置正常显示
	OLED_WR_Byte(0xA8, OLED_CMD); //--设置多路比例(1到64)
	OLED_WR_Byte(0x3f, OLED_CMD); //--1/64工作周期
	OLED_WR_Byte(0xD3, OLED_CMD); //-设置显示偏移	移动映射RAM计数器（0x00~0x3F）
	OLED_WR_Byte(0x00, OLED_CMD); //-无偏移
	OLED_WR_Byte(0xd5, OLED_CMD); //--设置显示时钟分频比/振荡频率
	OLED_WR_Byte(0x80, OLED_CMD); //--设置分频比，设置时钟为100帧/秒
	OLED_WR_Byte(0xD9, OLED_CMD); //--设置预充电周期
	OLED_WR_Byte(0xF1, OLED_CMD); // 设置预充电为15个时钟周期&放电为1个时钟周期
	OLED_WR_Byte(0xDA, OLED_CMD); //--设置com引脚硬件配置
	OLED_WR_Byte(0x12, OLED_CMD);
	OLED_WR_Byte(0xDB, OLED_CMD); //--设置vcomh
	OLED_WR_Byte(0x40, OLED_CMD); // 设置VCOM去选择电平
	OLED_WR_Byte(0x20, OLED_CMD); //-设置页面寻址模式（0x00/0x01/0x02）
	OLED_WR_Byte(0x02, OLED_CMD); //
	OLED_WR_Byte(0x8D, OLED_CMD); //--设置充电泵启用/禁用
	OLED_WR_Byte(0x14, OLED_CMD); //--设置(0x10)禁用
	OLED_WR_Byte(0xA4, OLED_CMD); // 禁用整个显示屏开启（0xa4/0xa5）
	OLED_WR_Byte(0xA6, OLED_CMD); // 禁用反向显示开启（0xa6/a7）
	OLED_Clear();
	OLED_Refresh();
	OLED_WR_Byte(0xAF, OLED_CMD);
	memset(rx_buff,0,256);
}

/**
 * ****************************************************************************************
 * @brief 改变屏幕亮度
 * @param level 亮度等级（0~255）
 * ****************************************************************************************
 */
void OELD_ChangeLuminance(u8 level)
{
	// 判断输入值合法性
	if (level > 255)
		level = 255;
	else if (level < 1)
		level = 1;
	// 配置OLED寄存器
	OLED_WR_Byte(0x81, OLED_CMD);
	OLED_WR_Byte(level, OLED_CMD);
}

u8 OLED_Backup[128][8]; ///< 备份缓冲区
/**
 * ****************************************************************************************
 * @brief 保存缓冲区（截图）
 * ****************************************************************************************
 */
void OLED_BufferSave(void)
{
	for (u8 i = 0; i < 128; i++)
		for (u8 j = 0; j < 8; j++)
			OLED_Backup[i][j] = OLED_GRAM[i][j];
}

/**
 * ****************************************************************************************
 * @brief 重载缓冲区
 * ****************************************************************************************
 */
void OLED_BufferRegain(void)
{
	for (u8 i = 0; i < 128; i++)
		for (u8 j = 0; j < 8; j++)
			OLED_GRAM[i][j] = OLED_Backup[i][j];
}
