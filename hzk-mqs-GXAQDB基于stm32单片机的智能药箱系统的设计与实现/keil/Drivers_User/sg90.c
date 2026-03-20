#include "sg90.h"


TIM_HandleTypeDef sg90;


// 0°舵机初始化
void sg90_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  sg90.Instance = sg90_tim;
  sg90.Init.Prescaler = 72-1;
  sg90.Init.CounterMode = TIM_COUNTERMODE_UP;
  sg90.Init.Period = 20000-1;
  sg90.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  sg90.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&sg90) != HAL_OK)
  {
   
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&sg90, &sMasterConfig) != HAL_OK)
  {
   
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&sg90, &sConfigOC, sg90_CHANNEL) != HAL_OK)
  {
    
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  sg90_MspPostInit(&sg90);
  
      // 启动PWM输出
    HAL_TIM_PWM_Start(&sg90, sg90_CHANNEL);  // 根据实际使用的通道替换
    // 初始角度设置为0度
    set_servo_angle(0);

}
//mspPost 初始化
void sg90_MspPostInit(TIM_HandleTypeDef* htim)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(htim->Instance==sg90_tim)
  {
    /* USER CODE BEGIN TIM3_MspPostInit 0 */

    /* USER CODE END TIM3_MspPostInit 0 */

    sg90GPIO_CLK_ENABLE();
    /**TIM3 GPIO Configuration
    PB1     ------> TIM3_CH4
    */
    GPIO_InitStruct.Pin = sg90_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(sg90_PORT, &GPIO_InitStruct);

    /* USER CODE BEGIN TIM3_MspPostInit 1 */

    /* USER CODE END TIM3_MspPostInit 1 */
  }

}

/* @brief TIM_PWM MSP Initialization
  * This function configures the hardware resources used in this example
  * @param htim_pwm: TIM_PWM handle pointer
  * @retval None
  */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{
  if(htim_pwm->Instance==sg90_tim)
  {
    /* USER CODE BEGIN TIM3_MspInit 0 */

    /* USER CODE END TIM3_MspInit 0 */
    /* Peripheral clock enable */
    sg90_tim_CLK_ENABLE();
    /* USER CODE BEGIN TIM3_MspInit 1 */

    /* USER CODE END TIM3_MspInit 1 */

  }

}


// 舵机角度控制函数
void set_servo_angle(uint8_t angle) 
{
    uint32_t pulse_width;

    // 舵机角度范围一般为0 - 180度
    if (angle > 180) {
        angle = 180;
    }

    // 计算脉冲宽度，SG90舵机的脉冲宽度范围一般为0.5ms - 2.5ms
    // PWM周期为20ms（50Hz），定时器分辨率为1000
    pulse_width = (angle * 2000 / 180) + 500;

    // 设置PWM占空比
    __HAL_TIM_SET_COMPARE(&sg90, sg90_CHANNEL, pulse_width);
}

