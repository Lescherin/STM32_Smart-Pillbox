#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>
#include "OLED_Data.h"

/**
 * @brief OLED显示屏功能用途：
 * - 实时显示传感器采集的数据
 * - 显示当前时间和日期
 * - 显示系统状态信息
 * - 提供直观的本地监控界面
 * 
 * @brief 引脚连接：
 * - 数据引脚（SDA）：PB9
 * - 时钟引脚（SCL）：PB8
 * 
 * @brief 通信方式：
 * - I2C串行通信（波特率：100kHz）
 * - 支持128x64分辨率
 * - 支持中英文显示
 */

/*参数宏定义*********************/

/*FontSize参数取值*/
/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
#define OLED_8X16			8
#define OLED_6X8			6

/*IsFilled参数数值*/
#define OLED_UNFILLED		0
#define OLED_FILLED			1

/*********************参数宏定义*/


/*函数声明*********************/

/**
 * @brief OLED初始化函数
 */
void OLED_Init(void);

/**
 * @brief 更新OLED显示
 */
void OLED_Update(void);

/**
 * @brief 更新OLED指定区域显示
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param Width 区域宽度
 * @param Height 区域高度
 */
void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

/**
 * @brief 清空OLED显存
 */
void OLED_Clear(void);

/**
 * @brief 清空OLED指定区域显存
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param Width 区域宽度
 * @param Height 区域高度
 */
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

/**
 * @brief 反转OLED显示
 */
void OLED_Reverse(void);

/**
 * @brief 反转OLED指定区域显示
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param Width 区域宽度
 * @param Height 区域高度
 */
void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

//字大点OLED_8X16
//OLED_6X8字小点

///*在(60, 38)位置显示浮点数字123.45，整数部分长度为3，小数部分长度为2，字体大小为6*8点阵*/
//	OLED_ShowFloatNum(60, 38, 123.45, 3, 2, OLED_6X8);
//
//	/*在(0, 48)位置显示英文和汉字串"Hello,世界。"，支持中英文混写*/
//	OLED_ShowString(0, 48, "Hello,世界。", OLED_8X16);

/*显示函数*/

/**
 * @brief 显示单个字符
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param Char 要显示的字符
 * @param FontSize 字体大小（OLED_8X16或OLED_6X8）
 */
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize);

/**
 * @brief 显示字符串
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param String 要显示的字符串指针
 * @param FontSize 字体大小（OLED_8X16或OLED_6X8）
 */
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize);

/**
 * @brief 显示无符号数字
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param Number 要显示的数字
 * @param Length 数字长度
 * @param FontSize 字体大小（OLED_8X16或OLED_6X8）
 */
void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);

/**
 * @brief 显示有符号数字
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param Number 要显示的数字
 * @param Length 数字长度
 * @param FontSize 字体大小（OLED_8X16或OLED_6X8）
 */
void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);

/**
 * @brief 显示十六进制数字
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param Number 要显示的数字
 * @param Length 数字长度
 * @param FontSize 字体大小（OLED_8X16或OLED_6X8）
 */
void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);

/**
 * @brief 显示二进制数字
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param Number 要显示的数字
 * @param Length 数字长度
 * @param FontSize 字体大小（OLED_8X16或OLED_6X8）
 */
void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);

/**
 * @brief 显示浮点数
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param Number 要显示的浮点数
 * @param IntLength 整数部分长度
 * @param FraLength 小数部分长度
 * @param FontSize 字体大小（OLED_8X16或OLED_6X8）
 */
void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);

/**
 * @brief 显示图像
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param Width 图像宽度
 * @param Height 图像高度
 * @param Image 图像数据指针
 */
void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);

/**
 * @brief 格式化显示
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param FontSize 字体大小（OLED_8X16或OLED_6X8）
 * @param format 格式化字符串
 * @param ... 可变参数列表
 */
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...);

/*绘图函数*/

/**
 * @brief 绘制点
 * @param X X坐标
 * @param Y Y坐标
 */
void OLED_DrawPoint(int16_t X, int16_t Y);

/**
 * @brief 获取指定点的状态
 * @param X X坐标
 * @param Y Y坐标
 * @retval 点的状态（1-点亮，0-熄灭）
 */
uint8_t OLED_GetPoint(int16_t X, int16_t Y);

/**
 * @brief 绘制直线
 * @param X0 起始点X坐标
 * @param Y0 起始点Y坐标
 * @param X1 结束点X坐标
 * @param Y1 结束点Y坐标
 */
void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1);

/**
 * @brief 绘制矩形
 * @param X 起始X坐标
 * @param Y 起始Y坐标
 * @param Width 矩形宽度
 * @param Height 矩形高度
 * @param IsFilled 是否填充（OLED_FILLED或OLED_UNFILLED）
 */
void OLED_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);

/**
 * @brief 绘制三角形
 * @param X0 第一个顶点X坐标
 * @param Y0 第一个顶点Y坐标
 * @param X1 第二个顶点X坐标
 * @param Y1 第二个顶点Y坐标
 * @param X2 第三个顶点X坐标
 * @param Y2 第三个顶点Y坐标
 * @param IsFilled 是否填充（OLED_FILLED或OLED_UNFILLED）
 */
void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled);

/**
 * @brief 绘制圆形
 * @param X 圆心X坐标
 * @param Y 圆心Y坐标
 * @param Radius 半径
 * @param IsFilled 是否填充（OLED_FILLED或OLED_UNFILLED）
 */
void OLED_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled);

/**
 * @brief 绘制椭圆
 * @param X 圆心X坐标
 * @param Y 圆心Y坐标
 * @param A 长半轴
 * @param B 短半轴
 * @param IsFilled 是否填充（OLED_FILLED或OLED_UNFILLED）
 */
void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);

/**
 * @brief 绘制圆弧
 * @param X 圆心X坐标
 * @param Y 圆心Y坐标
 * @param Radius 半径
 * @param StartAngle 起始角度
 * @param EndAngle 结束角度
 * @param IsFilled 是否填充（OLED_FILLED或OLED_UNFILLED）
 */
void OLED_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);

/*********************函数声明*/

/**
 * @brief OLED测试函数
 */
void oled_test(void);

#endif



/*****************江协科技|版权所有****************/
/*****************jiangxiekeji.com*****************/
