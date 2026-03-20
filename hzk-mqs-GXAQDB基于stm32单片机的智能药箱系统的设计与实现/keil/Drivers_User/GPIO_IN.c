#include "GPIO_IN.H"


void GPIO_IN_Init(void)
{
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
//	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
////	
//	GPIO_InitStruct.Pull = GPIO_IN1_MODE_PULL;
//	GPIO_InitStruct.Pin  = GPIO_IN1_PIN;
//    HAL_GPIO_Init(GPIO_IN1_PORT, &GPIO_InitStruct);
////	
//	GPIO_InitStruct.Pull = GPIO_IN2_MODE_PULL;
//	GPIO_InitStruct.Pin  = GPIO_IN2_PIN;
//  HAL_GPIO_Init(GPIO_IN2_PORT, &GPIO_InitStruct);
//	
//	GPIO_InitStruct.Pull = GPIO_IN3_MODE_PULL;
//	GPIO_InitStruct.Pin  = GPIO_IN3_PIN;
//  HAL_GPIO_Init(GPIO_IN3_PORT, &GPIO_InitStruct);
	
}

