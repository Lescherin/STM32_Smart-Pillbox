#ifndef __GPIO_IN_H
#define __GPIO_IN_H
#include "stm32f1xx_hal.h"

//时钟初始化已经在cubemx中初始化   GPIO_INX_MODE_PULL 可以选择1.GPIO_NOPULL 浮空输入  2.GPIO_PULLUP 下拉输入  3.GPIO_PULLDOWN  上拉输入
#define GPIO_IN1_PIN       			GPIO_PIN_14
#define GPIO_IN1_PORT      			GPIOB
#define GPIO_IN1_MODE_PULL      GPIO_NOPULL

#define GPIO_IN2_PIN       			GPIO_PIN_13
#define GPIO_IN2_PORT      			GPIOB
#define GPIO_IN2_MODE_PULL      GPIO_NOPULL

#define GPIO_IN3_PIN       			GPIO_PIN_4
#define GPIO_IN3_PORT    			  GPIOB
#define GPIO_IN3_MODE_PULL      GPIO_NOPULL


/* IO操作函数 */
#define GPIO_IN1_Detect         HAL_GPIO_ReadPin(GPIO_IN1_PORT, GPIO_IN1_PIN)  /* 数据端口输入 */
#define GPIO_IN2_Detect         HAL_GPIO_ReadPin(GPIO_IN2_PORT, GPIO_IN2_PIN)  /* 数据端口输入 */
#define GPIO_IN3_Detect         HAL_GPIO_ReadPin(GPIO_IN3_PORT, GPIO_IN3_PIN)  /* 数据端口输入 */


void GPIO_IN_Init(void);
#endif


