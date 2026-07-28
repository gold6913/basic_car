/**
 * @file oled.c
 * @author maker114
 * @brief OLED基础显示函数
 * @version STM32F407(软件SPI) 2.2
 * @date 2024-02-12
 */
#include "oled.h"
#include "oledfont.h"

/**
 * ****************************************************************************************
 * @brief 画线函数
 * @param x1 起点x坐标
 * @param y1 起点y坐标
 * @param x2 终点x坐标
 * @param y2 终点y坐标
 * @param mode 0:反显 1:正显
 * ****************************************************************************************
 */
void OLED_DrawLine(u8 x1, u8 y1, u8 x2, u8 y2, u8 mode)
{
	u16 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; // 计算坐标增量
	delta_y = y2 - y1;
	uRow = x1; // 画线起点坐标
	uCol = y1;
	incx = (delta_x > 0) - (delta_x < 0); // 精简设置单步方向
	incy = (delta_y > 0) - (delta_y < 0); // 精简设置单步方向
	delta_x = abs(delta_x);				  // 取绝对值，简化条件判断
	delta_y = abs(delta_y);
	distance = MAX(delta_x, delta_y); // 选取基本增量坐标轴

	for (t = 0; t <= distance; t++)
	{
		OLED_DrawPoint(uRow, uCol, mode); // 画点

		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}

/**
 * ****************************************************************************************
 * @brief 画虚线函数
 * @param x1 起点x坐标
 * @param y1 起点y坐标
 * @param x2 终点x坐标
 * @param y2 终点y坐标
 * @param mode 0:反显 1:正显
 * ****************************************************************************************
 */
void OLED_DrawDashedLine(u8 x1, u8 y1, u8 x2, u8 y2, u8 mode)
{
	u16 t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int Interval = 0, Polarity = 0;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; // 计算坐标增量
	delta_y = y2 - y1;
	uRow = x1; // 画线起点坐标
	uCol = y1;
	incx = (delta_x > 0) - (delta_x < 0); // 精简设置单步方向
	incy = (delta_y > 0) - (delta_y < 0); // 精简设置单步方向
	delta_x = abs(delta_x);				  // 取绝对值，简化条件判断
	delta_y = abs(delta_y);
	distance = MAX(delta_x, delta_y); // 选取基本增量坐标轴

	for (t = 0; t <= distance; t++)
	{
		Interval++;
		if (Interval <= 1)
		{
			// 通过条件运算符直接决定 OLED_DrawPoint 的第三个参数
			OLED_DrawPoint(uRow, uCol, Polarity == 0 ? 1 : 0);
		}
		else
		{
			Interval = 0;
			Polarity = !Polarity; // 颠倒 Polarity 的值
		}

		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}
/**
 * ****************************************************************************************
 * @brief 画空心圆
 * @param x 圆心x坐标
 * @param y 圆心y坐标
 * @param r 圆的半径
 * ****************************************************************************************
 */
void OLED_DrawCircle(int usX_Center, int usY_Center, int usRadius)
{
	short sCurrentX, sCurrentY;
	short sError;
	sCurrentX = 0;
	sCurrentY = usRadius;
	sError = 3 - (usRadius << 1); // 判断下个点位置的标志
	while (sCurrentX <= sCurrentY)
	{
		OLED_DrawPoint(usX_Center + sCurrentX, usY_Center + sCurrentY, 1); // 1，研究对象
		OLED_DrawPoint(usX_Center - sCurrentX, usY_Center + sCurrentY, 1); // 2
		OLED_DrawPoint(usX_Center - sCurrentY, usY_Center + sCurrentX, 1); // 3
		OLED_DrawPoint(usX_Center - sCurrentY, usY_Center - sCurrentX, 1); // 4
		OLED_DrawPoint(usX_Center - sCurrentX, usY_Center - sCurrentY, 1); // 5
		OLED_DrawPoint(usX_Center + sCurrentX, usY_Center - sCurrentY, 1); // 6
		OLED_DrawPoint(usX_Center + sCurrentY, usY_Center - sCurrentX, 1); // 7
		OLED_DrawPoint(usX_Center + sCurrentY, usY_Center + sCurrentX, 1); // 0
		sCurrentX++;
		if (sError < 0)
			sError += 4 * sCurrentX + 6;
		else
		{
			sError += 10 + 4 * (sCurrentX - sCurrentY);
			sCurrentY--;
		}
	}
}

#define PI 3.1415926535
/**
 * ****************************************************************************************
 * @brief 画空心圆（三角函数版）
 * @param x 圆心x坐标
 * @param y 圆心y坐标
 * @param r 圆的半径
 * ****************************************************************************************
 */
void OLED_DrawCircle_math(u8 x, u8 y, u8 r)
{
	float Deviation_x = 0, Deviation_y = 0;
	for (u8 i = 1; i < 90; i++)
	{
		if (i == 45)
			continue;

		Deviation_x = r * cos(i * 2 * PI / 360);
		Deviation_y = r * sin(i * 2 * PI / 360);

		// 第一象限
		OLED_DrawPoint(x + Deviation_x, y + Deviation_y, 1);
		// 第二象限
		OLED_DrawPoint(x - Deviation_x, y + Deviation_y, 1);
		// 第三象限
		OLED_DrawPoint(x + Deviation_x, y - Deviation_y, 1);
		// 第四象限
		OLED_DrawPoint(x - Deviation_x, y - Deviation_y, 1);
	}
}

/**
 * ****************************************************************************************
 * @brief 在指定位置显示一个字符,包括部分字符
 * @param x 0~127
 * @param y 0~63
 * @param chr 文本内容
 * @param size1 选择字体 6x8/6x12/8x16/12x24
 * @param mode 0,反色显示;1,正常显示
 * ****************************************************************************************
 */
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 size1, u8 mode)
{
	u8 i, m, temp, size2, chr1;
	u8 x0 = x, y0 = y;
	if (size1 == 8)
		size2 = 6;
	else
		size2 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * (size1 / 2); // 得到字体一个字符对应点阵集所占的字节数
	chr1 = chr - ' ';											   // 计算偏移后的值
	for (i = 0; i < size2; i++)
	{
		if (size1 == 8)
		{
			temp = asc2_0806[chr1][i];
		} // 调用0806字体
		else if (size1 == 12)
		{
			temp = asc2_1206[chr1][i];
		} // 调用1206字体
		else if (size1 == 16)
		{
			temp = asc2_1608[chr1][i];
		} // 调用1608字体
		else if (size1 == 24)
		{
			temp = asc2_2412[chr1][i];
		} // 调用2412字体
		else
			return;
		for (m = 0; m < 8; m++)
		{
			if (temp & 0x01)
				OLED_DrawPoint(x, y, mode);
			else
				OLED_DrawPoint(x, y, !mode);
			temp >>= 1;
			y++;
		}
		x++;
		if ((size1 != 8) && ((x - x0) == size1 / 2))
		{
			x = x0;
			y0 = y0 + 8;
		}
		y = y0;
	}
}

/**
 * ****************************************************************************************
 * @brief 显示一个字符串
 * @param x x起点坐标
 * @param y y起点坐标
 * @param chr 字符串起始地址
 * @param size1 字体大小
 * @param mode 0,反色显示;1,正常显示
 * ****************************************************************************************
 */
void OLED_ShowString(u8 x, u8 y, u8 *chr, u8 size1, u8 mode)
{
	while ((*chr >= ' ') && (*chr <= '~')) // 判断是不是非法字符!
	{
		OLED_ShowChar(x, y, *chr, size1, mode);
		if (size1 == 8)
			x += 6;
		else
			x += size1 / 2;
		chr++;
	}
}

// m^n
u32 OLED_Pow(u8 m, u8 n)
{
	u32 result = 1;
	while (n--)
	{
		result *= m;
	}
	return result;
}

/**
 * ****************************************************************************************
 * @brief 显示数字
 * @param x x起点坐标
 * @param y y起点坐标
 * @param num 要显示的数字
 * @param len 数字的位数
 * @param size1 字体大小
 * @param mode 0,反色显示;1,正常显示
 * ****************************************************************************************
 */
void OLED_ShowNum(u8 x, u8 y, int num, u8 len, u8 size1, u8 mode)
{
	u8 t, temp, next_temp, first_flag = 0, m = 0;
	if (size1 == 8)
		m = 2;
	for (t = 0; t < len; t++)
	{
		temp = (num / OLED_Pow(10, len - t - 1)) % 10; // 当t=0时显示千位数，以此类推
		next_temp = (num / OLED_Pow(10, len - t)) % 10;
		if (temp == 0)
		{
			if (next_temp == 0 && first_flag == 0) // 下一个数字不是0且还没开始显示数字
			{
				OLED_ShowChar(x + (size1 / 2 + m) * t, y, ' ', size1, mode);
			}
			if (first_flag == 1 || t == len - 1)
			{
				OLED_ShowChar(x + (size1 / 2 + m) * t, y, '0', size1, mode);
				first_flag = 1;
			}
		}
		else
		{
			OLED_ShowChar(x + (size1 / 2 + m) * t, y, temp + '0', size1, mode);
			first_flag = 1;
		}
	}
}
/**
 * ****************************************************************************************
 * @brief 显示带负数数字
 * @param x x起点坐标
 * @param y y起点坐标
 * @param num 要显示的数字
 * @param len 数字的位数
 * @param size1 字体大小
 * @param mode 0,反色显示;1,正常显示
 * ****************************************************************************************
 */
void OLED_ShowSNum(u8 x, u8 y, int num, u8 len, u8 size1, u8 mode)
{
	u8 t, temp, next_temp, first_flag = 0, m = 0;
	if (size1 == 8)
		m = 2;
	if (num < 0)
	{
		// OLED_ShowString(x, y, '-', size1, mode);
		OLED_ShowChar(x, y, '-', size1, mode);
		num = -num;
	}
	else OLED_ShowChar(x, y, ' ', size1, mode);
		
	x += size1 / 2 + m;
	for (t = 0; t < len; t++)
	{
		temp = (num / OLED_Pow(10, len - t - 1)) % 10; // 当t=0时显示千位数，以此类推
		next_temp = (num / OLED_Pow(10, len - t)) % 10;
		if (temp == 0)
		{
			if (next_temp == 0 && first_flag == 0) // 下一个数字不是0且还没开始显示数字
			{
				OLED_ShowChar(x + (size1 / 2 + m) * t, y, ' ', size1, mode);
			}
			if (first_flag == 1 || t == len - 1)
			{
				OLED_ShowChar(x + (size1 / 2 + m) * t, y, '0', size1, mode);
				first_flag = 1;
			}
		}
		else
		{
			OLED_ShowChar(x + (size1 / 2 + m) * t, y, temp + '0', size1, mode);
			first_flag = 1;
		}
	}
}
/**
 * ****************************************************************************************
 * @brief 显示汉字
 * @param x x起点坐标
 * @param y y起点坐标
 * @param num 汉字对应的序号
 * @param size1 字体大小
 * @param mode 0,反色显示;1,正常显示
 * ****************************************************************************************
 */
void OLED_ShowChinese(u8 x, u8 y, u8 num, u8 size1, u8 mode)
{
	u8 m, temp;
	u8 x0 = x, y0 = y;
	u16 i, size3 = (size1 / 8 + ((size1 % 8) ? 1 : 0)) * size1; // 得到字体一个字符对应点阵集所占的字节数
	for (i = 0; i < size3; i++)
	{
		if (size1 == 16)
		{
			temp = Hzk1[num][i];
		} // 调用16*16字体
		else if (size1 == 24)
		{
			temp = Hzk2[num][i];
		} // 调用24*24字体
		else if (size1 == 32)
		{
			temp = Hzk3[num][i];
		} // 调用32*32字体
		else if (size1 == 64)
		{
			temp = Hzk4[num][i];
		} // 调用64*64字体
		else
			return;
		for (m = 0; m < 8; m++)
		{
			if (temp & 0x01)
				OLED_DrawPoint(x, y, mode);
			else
				OLED_DrawPoint(x, y, !mode);
			temp >>= 1;
			y++;
		}
		x++;
		if ((x - x0) == size1)
		{
			x = x0;
			y0 = y0 + 8;
		}
		y = y0;
	}
}

/**
 * ****************************************************************************************
 * @brief 显示图片
 * @param x x起点坐标
 * @param y y起点坐标
 * @param sizex 图片长度
 * @param sizey 图片宽度
 * @param BMP 要写入的图片数组
 * @param mode 反色显示;1,正常显示
 * ****************************************************************************************
 */
void OLED_ShowPicture(u8 x, u8 y, u8 sizex, u8 sizey, u8 BMP[], u8 mode)
{
	u16 j = 0;
	u8 i, n, temp, m;
	u8 x0 = x, y0 = y;
	sizey = sizey / 8 + ((sizey % 8) ? 1 : 0);
	for (n = 0; n < sizey; n++)
	{
		for (i = 0; i < sizex; i++)
		{
			temp = BMP[j];
			j++;
			for (m = 0; m < 8; m++)
			{
				if (temp & 0x01)
					OLED_DrawPoint(x, y, mode);
				else
					OLED_DrawPoint(x, y, !mode);
				temp >>= 1;
				y++;
			}
			x++;
			if ((x - x0) == sizex)
			{
				x = x0;
				y0 = y0 + 8;
			}
			y = y0;
		}
	}
}

/**
 * @}
 */

/*======================================================额外图形库=====================================================*/
/** @addtogroup 额外图形库
 * @{
 */

/**
 * ****************************************************************************************
 * @brief 快速水平线
 * @param x x起点坐标
 * @param y y起点坐标
 * @param w 宽度（长度）
 * @param mode 1 填充 0,清空
 * ****************************************************************************************
 */
void OLED_DrawFastHLine(int x, int y, int w, int mode)
{
	int end = x + w;
	int a;
	for (a = MAX(0, x); a < MIN(end, 128); a++)
	{
		OLED_DrawPoint(a, y, mode);
	}
}

/**
 * ****************************************************************************************
 * @brief 快速垂直线
 * @param x x起点坐标
 * @param y y起点坐标
 * @param h 高度（长度）
 * @param mode 1 填充 0,清空
 * ****************************************************************************************
 */
void OLED_DrawFastVLine(int x, int y, int h, int mode)
{
	int end = y + h;
	int a;
	for (a = MAX(0, y); a < MIN(end, 64); a++)
	{
		OLED_DrawPoint(x, a, mode);
	}
}

/**
 * ****************************************************************************************
 * @brief 绘制填充矩形
 * @param x x起点坐标
 * @param y y起点坐标
 * @param w 宽度
 * @param h 高度
 * @param mode 1 填充 0,清空
 * ****************************************************************************************
 */
void OLED_DrawFill(int x, int y, int w, int h, int mode)
{
	int end_x = x + w;
	int end_y = y + h;
	int a, b;
	for (a = MAX(0, x); a < MIN(end_x, 128); a++)
	{
		for (b = MAX(0, y); b < MIN(end_y, 64); b++)
		{
			OLED_DrawPoint(a, b, mode);
		}
	}
}

/**
 * ****************************************************************************************
 * @brief 绘制指定线宽矩形
 * @param x x起点坐标
 * @param y y起点坐标
 * @param w 宽度
 * @param h 高度
 * @param Thickness 宽度（厚度）
 * @param mode 1 填充 0,清空
 * @note 线的粗细向内扩展
 * ****************************************************************************************
 */
void OLED_DrawFill_T(int x, int y, int w, int h, u8 Thickness, int mode)
{
	for (u8 i = 0; i < Thickness; i++)
	{
		// 绘制左竖线
		OLED_DrawFastVLine(x + i, y + i, h - 2 * i, mode);
		// 绘制上横线
		OLED_DrawFastHLine(x + i, y + i, w - 2 * i, mode);
		// 绘制下横线
		OLED_DrawFastHLine(x + i, y - i + h, w - 2 * i, mode);
		// 绘制右竖线
		OLED_DrawFastVLine(x - i + w, y + i, h + 1 - 2 * i, mode);
	}
}

/**
 * ****************************************************************************************
 * @brief 绘制空心矩形
 * @param x x起点坐标
 * @param y y起点坐标
 * @param w 宽度
 * @param h 高度
 * @param mode 1 填充 0,清空
 * ****************************************************************************************
 */
void OLED_DrawBox(int x, int y, int w, int h, int mode)
{
	// 绘制四条线段
	// 绘制左竖线
	OLED_DrawFastVLine(x, y, h, mode);
	// 绘制上横线
	OLED_DrawFastHLine(x, y, w, mode);
	// 绘制下横线
	OLED_DrawFastHLine(x, y + h, w, mode);
	// 绘制右竖线
	OLED_DrawFastVLine(x + w, y, h + 1, mode);
}

/**
 * ****************************************************************************************
 * @brief 绘制空圆角矩形
 * @param x x起点坐标
 * @param y y起点坐标
 * @param w 宽度
 * @param h 高度
 * @param r 圆角半径
 * @param mode 1 填充 0,清空
 * ****************************************************************************************
 */
void OLED_DrawRoundedBox(int x, int y, int w, int h, int r, int mode)
{
	short sCurrentX, sCurrentY;
	short sError;

	// 绘制圆角
	sCurrentX = 0;
	sCurrentY = r;
	sError = 3 - (r << 1); // 判断下个点位置的标志

	while (sCurrentX <= sCurrentY)
	{
		// 右上角
		OLED_DrawPoint(x + w - r + sCurrentX, y + r - sCurrentY, mode); // 6
		OLED_DrawPoint(x + w - r + sCurrentY, y + r - sCurrentX, mode); // 7

		// 左上角
		OLED_DrawPoint(x + r - sCurrentX, y + r - sCurrentY, mode); // 5
		OLED_DrawPoint(x + r - sCurrentY, y + r - sCurrentX, mode); // 4

		// 左下角
		OLED_DrawPoint(x + r - sCurrentX, y + h - r + sCurrentY, mode); // 1
		OLED_DrawPoint(x + r - sCurrentY, y + h - r + sCurrentX, mode); // 2

		// 右下角
		OLED_DrawPoint(x + w - r + sCurrentX, y + h - r + sCurrentY, mode); // 0
		OLED_DrawPoint(x + w - r + sCurrentY, y + h - r + sCurrentX, mode); // 3

		sCurrentX++;
		if (sError < 0)
		{
			sError += 4 * sCurrentX + 6;
		}
		else
		{
			sError += 10 + 4 * (sCurrentX - sCurrentY);
			sCurrentY--;
		}
	}
	// 绘制上边水平直线
	OLED_DrawFastHLine(x + r, y, w - 2 * r, 1);
	// 绘制下边水平直线
	OLED_DrawFastHLine(x + r, y + h, w - 2 * r, 1);
	// 绘制左边垂直直线
	OLED_DrawFastVLine(x, y + r, h - 2 * r, 1);
	// 绘制右边垂直直线
	OLED_DrawFastVLine(x + w, y + r, h - 2 * r, 1);
}

/**
 * ****************************************************************************************
 * @brief 十六进制显示函数
 * @param x x轴坐标
 * @param y y轴坐标
 * @param num 需要显示的数据（十进制）
 * @param size 字号
 * @param mode 1:加上“0x”  0:直接显示
 * @note
 * - 只能显示一个字节的十六进制无符号整数
 * - 默认正常显示而非反色
 * ****************************************************************************************
 */
void OLED_ShowNum16(u8 x, u8 y, u8 num, u8 size, u8 mode)
{
	// 判断是否显示“0x”（注意可能存在的显示空间不足的问题）
	if (mode == 1)
	{
		OLED_ShowString(x, y, "0x", size, 1);
		x += size;
	}
	// 将数字转换为十六进制后显示
	// 十位
	if (num / 16 < 10)
	{
		OLED_ShowNum(x, y, num / 16, 1, size, 1);
	}
	else
	{
		OLED_ShowChar(x, y, 65 + (int)(num / 16) - 10, size, 1);
	}
	// 个位
	x += size / 2;

	if (num % 16 < 10)
	{
		OLED_ShowNum(x, y, num % 16, 1, size, 1);
	}
	else
	{
		OLED_ShowChar(x, y, 65 + (int)(num % 16) - 10, size, 1);
	}
}

/**
 * ****************************************************************************************
 * @brief 绘制等距水平平行线
 * @param x x轴起始坐标
 * @param y y轴起始坐标
 * @param w 宽度
 * @param h 高度
 * @param section 分段（细分）数
 * @param mode 1 填充 0,清空
 * ****************************************************************************************
 */
void OLED_DrawParallelHLine(u8 x, u8 y, u8 w, u8 h, u8 section, u8 mode)
{
	// 判定输入值是否合法
	if (section < 2)
		return;
	// 确定各分段坐标间距
	float space = (float)h / (float)(section - 1);
	// 遍历显示每一条直线
	for (u8 i = 0; i < section; i++)
	{
		OLED_DrawFastHLine(x, y + i * space, w, mode);
	}
}

/**
 * ****************************************************************************************
 * @brief 绘制等距垂直平行线
 * @param x x轴起始坐标
 * @param y y轴起始坐标
 * @param w 宽度
 * @param h 高度
 * @param section 分段（细分）数
 * @param mode 1 填充 0,清空
 * ****************************************************************************************
 */
void OLED_DrawParallelVLine(u8 x, u8 y, u8 w, u8 h, u8 section, u8 mode)
{
	// 判定输入值是否合法
	if (section < 2)
		return;
	// 确定各分段坐标间距
	float space = (float)w / (float)(section - 1);
	// 遍历显示每一条直线
	for (u8 i = 0; i < section; i++)
	{
		OLED_DrawFastVLine(x + i * space, y, h, mode);
	}
}

/**
 * ****************************************************************************************
 * @brief 虚化目标区域
 * @param x x坐标起始点
 * @param y y坐标起始点
 * @param w 宽度
 * @param h 高度
 * ****************************************************************************************
 */
void OLED_DrawVacuumization(u8 x, u8 y, u8 w, u8 h)
{
#if 1
	// 棋盘状虚化
	for (u8 m = x; m < w; m++)
		for (u8 n = y; n < h; n++)
			if ((m % 2) == (n % 2))
				OLED_DrawPoint(m, n, 0);
#else
	// 网格状虚化
	for (u8 m = x; m < w; m++)
		if ((m % 2) == 1)
			OLED_DrawFastVLine(m, y, h, 0);
	for (u8 n = y; n < h; n++)
		if ((n % 2) == 1)
			OLED_DrawFastHLine(x, n, w, 0);
#endif
}

/**
 * ****************************************************************************************
 * @brief 显示两位小数
 * @param x x起点坐标
 * @param y y起点坐标
 * @param data 要显示的数字
 * @param len 整数长度
 * @param size 字体大小
 * @param mode 0,反色显示;1,正常显示
 * ****************************************************************************************
 */
void OLED_ShowFNum(u8 x, u8 y, float data, u8 len, u8 size, u8 mode)
{
	int m = (size == 8) ? 2 : 0;
	// 提纯与分离小数与整数
	int num = data * 100;
	int fnum = num % 100;
	int inum = num / 100;
	// 显示数字
	OLED_ShowNum(x, y, inum, len, size, 1);
	OLED_ShowString(x + (size / 2 + m) * len, y, ".", size, 1);
	OLED_ShowNum(x + (size / 2 + m) * (len + 1), y, fnum, 2, size, 1);

	if (fnum < 10)
		OLED_ShowChar(x + (size / 2 + m) * (len + 1), y, '0', size, 1);
}

/**
 * ****************************************************************************************
 * @brief 显示有负数的两位小数
 * @param x x起点坐标
 * @param y y起点坐标
 * @param data 要显示的数字
 * @param len 整数长度
 * @param size 字体大小
 * @param mode 0,反色显示;1,正常显示
 * ****************************************************************************************
 */
void OLED_ShowSFNum(u8 x, u8 y, float data, u8 len, u8 size, u8 mode)
{
	int m = (size == 8) ? 2 : 0;
	if (data < 0)
	{
		OLED_ShowString(x, y, "-", size, mode);
		data = -data;
	}
	else
	{
		OLED_ShowString(x, y, " ", size, mode);
	}
	x += size / 2 + m;
	// 提纯与分离小数与整数
	int num = data * 100;
	int fnum = num % 100;
	int inum = num / 100;
	// 显示数字
	OLED_ShowNum(x, y, inum, len, size, 1);
	OLED_ShowString(x + (size / 2 + m) * len, y, ".", size, 1);
	OLED_ShowNum(x + (size / 2 + m) * (len + 1), y, fnum, 2, size, 1);

	if (fnum < 10)
		OLED_ShowChar(x + (size / 2 + m) * (len + 1), y, '0', size, 1);
}

/**
 * ****************************************************************************************
 * @brief 绘制一条贝塞尔曲线
 * @param x0 端点A的x坐标
 * @param y0 端点A的y坐标
 * @param x1 控制点的x坐标
 * @param y1 控制点的y坐标
 * @param x2 端点B的x坐标
 * @param y2 端点B的y坐标
 * ****************************************************************************************
 */
void OLED_DrawBezierCurve(int x0, int y0, int x1, int y1, int x2, int y2)
{
	// 动态调节步长
	double a = abs(x2 - x0);
	double b = abs(y2 - y0);
	double step = 0.6 / sqrt(a * a + b * b);

	for (float t = 0; t <= 1; t += step)

	{
		// 根据二次贝塞尔曲线的公式计算 x 和 y 坐标
		int xt = (int)((1 - t) * (1 - t) * x0 + 2 * (1 - t) * t * x1 + t * t * x2);
		int yt = (int)((1 - t) * (1 - t) * y0 + 2 * (1 - t) * t * y1 + t * t * y2);

		// 使用 OLED_drawPoint 函数绘制计算出的点
		OLED_DrawPoint(xt, yt, 1);
	}
}

/**
 * ****************************************************************************************
 * @brief 绘制一个由贝塞尔曲线构成的G2圆角矩形
 * @param x 起点x坐标
 * @param y 起点y坐标
 * @param w 宽
 * @param h 高
 * @param r 圆角系数，建议为短边长度的40%左右
 * ****************************************************************************************
 */
void OLED_DrawBezierBox(int x, int y, int w, int h, int r)
{
	int x_start = x + r,
		x_end = x + w - r,
		y_start = y + r,
		y_end = y + h - r;
	/****直线部分****/
	// 上直线
	OLED_DrawFastHLine(x_start, y_start - r, w - 2 * r + 1, 1);
	// 下直线
	OLED_DrawFastHLine(x_start, y_end + r, w - 2 * r + 1, 1);
	// 左直线
	OLED_DrawFastVLine(x_start - r, y_start, h - 2 * r + 1, 1);
	// 右直线
	OLED_DrawFastVLine(x_end + r, y_start, h - 2 * r + 1, 1);
	/****曲线部分****/

	// 动态调节步长
	double step = 0.6 / sqrt(2 * r * r);

	for (float t = 0; t <= 1; t += step)
	{
		// 根据二次贝塞尔曲线的公式计算 x 和 y 坐标
		int xt = (int)((1 - t) * (1 - t) * 0 + 2 * (1 - t) * t * 0 + t * t * r);
		int yt = (int)((1 - t) * (1 - t) * 0 + 2 * (1 - t) * t * r + t * t * r);

		// 使用 OLED_drawPoint 函数绘制计算出的点
		OLED_DrawPoint(x_start - r + xt, y_start - yt - 1, 1);
		OLED_DrawPoint(x_end + r - xt, y_start - yt - 1, 1);
		OLED_DrawPoint(x_start - r + xt, y_end + yt + 1, 1);
		OLED_DrawPoint(x_end + r - xt, y_end + yt + 1, 1);
	}
}
