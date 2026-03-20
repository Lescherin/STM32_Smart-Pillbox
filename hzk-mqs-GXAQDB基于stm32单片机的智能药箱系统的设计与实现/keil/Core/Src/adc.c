/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "adc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;

// ADC初始化
void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PA0-WKUP     ------> ADC1_IN0
    PA1     ------> ADC1_IN1
    PA4     ------> ADC1_IN4
    PA5     ------> ADC1_IN5
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA0-WKUP     ------> ADC1_IN0
    PA1     ------> ADC1_IN1
    PA4     ------> ADC1_IN4
    PA5     ------> ADC1_IN5
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5);

  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/**
 * @brief       ����ADCͨ������ʱ��
 * @param       adcx : adc���ָ��,ADC_HandleTypeDef
 * @param       ch   : ͨ����, ADC_CHANNEL_0~ADC_CHANNEL_17
 * @param       stime: ����ʱ��  0~7, ��Ӧ��ϵΪ:
 *   @arg       ADC_SAMPLETIME_1CYCLE_5, 1.5��ADCʱ������        ADC_SAMPLETIME_7CYCLES_5, 7.5��ADCʱ������
 *   @arg       ADC_SAMPLETIME_13CYCLES_5, 13.5��ADCʱ������     ADC_SAMPLETIME_28CYCLES_5, 28.5��ADCʱ������
 *   @arg       ADC_SAMPLETIME_41CYCLES_5, 41.5��ADCʱ������     ADC_SAMPLETIME_55CYCLES_5, 55.5��ADCʱ������
 *   @arg       ADC_SAMPLETIME_71CYCLES_5, 71.5��ADCʱ������     ADC_SAMPLETIME_239CYCLES_5, 239.5��ADCʱ������
 * @param       rank: ��ͨ���ɼ�ʱ��Ҫ���õĲɼ����,
                �����㶨��channle1��rank=1��channle2 ��rank=2��
                ��ô��Ӧ����DMA����ռ�ı�������AdcDMA[0] ��i��channle1��ת�������AdcDMA[1]����ͨ��2��ת������� 
                ��ͨ��DMA����Ϊ ADC_REGULAR_RANK_1
 *   @arg       ���1~16��ADC_REGULAR_RANK_1~ADC_REGULAR_RANK_16
 * @retval      ��
 */
void adc_channel_set(ADC_HandleTypeDef *adc_handle, uint32_t ch, uint32_t rank, uint32_t stime)
{
    ADC_ChannelConfTypeDef adc_ch_conf;
    
    adc_ch_conf.Channel = ch;                            /* ͨ�� */
    adc_ch_conf.Rank = rank;                             /* ���� */
    adc_ch_conf.SamplingTime = stime;                    /* ����ʱ�� */
    HAL_ADC_ConfigChannel(adc_handle, &adc_ch_conf);     /* ͨ������ */
}

/**
 * @brief       ���ADCת����Ľ��
 * @param       ch: ͨ��ֵ 0~17��ȡֵ��ΧΪ��ADC_CHANNEL_0~ADC_CHANNEL_17
 * @retval      ��
 */
uint32_t adc_get_result(uint32_t ch)
{
    adc_channel_set(&hadc1 , ch, ADC_REGULAR_RANK_1, ADC_SAMPLETIME_239CYCLES_5);    /* ����ͨ�������кͲ���ʱ�� */

    HAL_ADC_Start(&hadc1);                            /* ����ADC */
    HAL_ADC_PollForConversion(&hadc1, 10);            /* ��ѯת�� */
    return (uint16_t)HAL_ADC_GetValue(&hadc1);        /* �������һ��ADC1�������ת����� */
}
/* USER CODE END 1 */
