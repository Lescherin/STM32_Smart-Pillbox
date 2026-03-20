#include "GPIO_OUT.h"

/**
 * @brief GPIO输出模块初始化
 * 
 * 初始化蜂鸣器、LED指示灯、继电器等GPIO输出设备
 * 配置GPIO为推挽输出模式，无上下拉电阻，高速模式
 * 初始状态：蜂鸣器关闭（低电平有效，此处设置为1表示关闭）
 */
void GPIO_OUT_Init(void)
{
    // 开启GPIOA和GPIOB时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 配置GPIO为推挽输出模式，无上下拉电阻，高速模式
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    // LED1初始化（当前注释掉）
//    GPIO_InitStruct.Pin  = LED1_PIN;
//    HAL_GPIO_Init(LED1_PORT, &GPIO_InitStruct);
//    LED1_OUT(1);

    // LED2初始化（当前注释掉）
//    GPIO_InitStruct.Pin  = LED2_PIN;
//    HAL_GPIO_Init(LED2_PORT, &GPIO_InitStruct);
//    LED2_OUT(1);

    // 蜂鸣器初始化
    GPIO_InitStruct.Pin  = BEEP_PIN;
    HAL_GPIO_Init(BEEP_PORT, &GPIO_InitStruct);
    BEEP_OUT(1);  // 初始状态：蜂鸣器关闭（低电平有效）

    // 继电器1初始化（当前注释掉）
//    GPIO_InitStruct.Pin  = JDQ1_PIN;
//    HAL_GPIO_Init(JDQ1_PORT, &GPIO_InitStruct);
//    JDQ1_OUT(1);
	
    // 继电器2初始化（当前注释掉）
//	GPIO_InitStruct.Pin  = JDQ2_PIN;
//    HAL_GPIO_Init(JDQ2_PORT, &GPIO_InitStruct);
//	JDQ2_OUT(1);
	
}


