#ifndef __HX711_H
#define __HX711_H

#include "main.h"

// 假设HX711的SCK连接到PA8引脚，DOUT连接到PA9引脚，可根据实际硬件连接修改
#define HX711_SCK_GPIO_Port     GPIOB
#define HX711_SCK_Pin           GPIO_PIN_10
#define HX711_DOUT_GPIO_Port    GPIOB
#define HX711_DOUT_Pin          GPIO_PIN_11

#define HX711_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

#define GapValue 368
//因为不同的传感器特性曲线不一样，因此，每一个传感器需要矫正这里的GapValue这个除数。
//当发现测试出来的重量偏大时，增加该数值。
//如果测试出来的重量偏小时，减小改数值。 

// 函数声明
void HX711_Init(void);
unsigned long HX711_Read(void);
int32_t Get_Weight(void);

#endif
