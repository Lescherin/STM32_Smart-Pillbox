#ifndef __sg90_H__
#define __sg90_H__

#include "main.h"

/**
 * @brief SG90舵机功能用途：
 * - 控制药箱出药口的开关
 * - 实现定时自动出药功能
 * - 响应云端控制指令
 * 
 * @brief 引脚连接：
 * - PWM控制引脚：PA7
 * 
 * @brief 通信方式：
 * - PWM脉冲宽度调制
 * - 50Hz PWM信号
 * - 脉冲宽度：0.5ms~2.5ms对应角度：0°~180°
 */

#define sg90_tim                            TIM3
#define sg90_tim_IRQn                       TIM3_IRQn
#define sg90_tim_IRQHandler                 TIM3_IRQHandler
#define sg90_tim_CLK_ENABLE()               do{ __HAL_RCC_TIM3_CLK_ENABLE(); }while(0)  /* TIM3 时钟使能 */
#define sg90_CHANNEL                        TIM_CHANNEL_2

#define sg90_PORT                           GPIOA
#define sg90_PIN                            GPIO_PIN_7
#define sg90GPIO_CLK_ENABLE()               do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)             /* PA口时钟使能 */

/**
 * @brief TIM_PWM MSP初始化后的配置
 * @param htim TIM_PWM句柄指针
 */
void sg90_MspPostInit(TIM_HandleTypeDef *htim);

/**
 * @brief SG90舵机初始化
 */
void sg90_Init(void);

/**
 * @brief 设置舵机角度
 * @param angle 舵机角度（0°~180°）
 */
void set_servo_angle(uint8_t angle);

#endif



