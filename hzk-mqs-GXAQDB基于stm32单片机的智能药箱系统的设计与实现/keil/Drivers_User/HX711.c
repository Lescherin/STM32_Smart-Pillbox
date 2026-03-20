#include "HX711.h"
#include "delay.h"  /* 需要包含延时函数头文件 */

/**
HX711称重传感器
功能用途 ：

- 测量药箱中药品的重量
- 可用于检测药品是否被取出或放入
- 通过重量变化判断药品使用情况
引脚连接 ：

- 时钟引脚（SCK）： PB10
- 数据引脚（DOUT）： PB11
通信方式 ：

- 2线串行接口（时钟+数据）
- 上升沿接收数据，24位数据格式
- 第25个时钟脉冲用于设置增益和通道选择
*/

uint32_t Weight_Maopi;

// 初始化HX711相关GPIO
void HX711_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 使能GPIO时钟

    HX711_GPIO_CLK_ENABLE();

    // 配置SCK引脚为输出
    GPIO_InitStruct.Pin = HX711_SCK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(HX711_SCK_GPIO_Port, &GPIO_InitStruct);
    // 配置DOUT引脚为输入
    GPIO_InitStruct.Pin = HX711_DOUT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(HX711_DOUT_GPIO_Port, &GPIO_InitStruct);
    delay_ms(100);

    Weight_Maopi = HX711_Read();        //获取毛皮重量
}

// 读取HX711数据
unsigned long HX711_Read(void)
{
    uint32_t data = 0;
    uint32_t i;

    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);
  
    while(HAL_GPIO_ReadPin(HX711_DOUT_GPIO_Port, HX711_DOUT_Pin) == GPIO_PIN_SET);

    for(i = 0; i < 24; i++)
    {
        HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);
        data = data << 1;
        HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);
        if(HAL_GPIO_ReadPin(HX711_DOUT_GPIO_Port, HX711_DOUT_Pin) == GPIO_PIN_SET)
        {
            data++;
        }
    }

    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);
    data = data ^ 0x800000; // 第25个脉冲下降沿来时，转换数据
    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);

    return data;
}



//获取称重数据

int32_t Get_Weight(void)
{
    // 读取HX711传感器数据
    const int32_t currentReading = HX711_Read();
    
    // 初始化结果变量
    int32_t objectWeight = 0;
    
    // 仅当读取值大于皮重时计算重量
    if (currentReading > Weight_Maopi)
    {
        // 计算物体的原始AD差值
        const int32_t adcDifference = currentReading - Weight_Maopi;
        
        // 应用校准系数转换为实际重量
        objectWeight = (int32_t)((float)adcDifference / GapValue);
    }
    
    return objectWeight;
}
