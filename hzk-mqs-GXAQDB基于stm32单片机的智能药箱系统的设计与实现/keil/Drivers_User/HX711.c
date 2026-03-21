#include "HX711.h"
#include "delay.h"  /* 需要包含延时函数头文件 */

/**
HX711称重传感器驱动模块
功能用途：
- 测量药箱中药品的实时重量
- 通过重量变化检测药品的取出或放入操作
- 为智能药箱的药品管理提供数据支持

硬件连接：
- 时钟引脚（SCK）： PB10
- 数据引脚（DOUT）： PB11

通信协议：
- 2线串行接口（时钟+数据）
- 上升沿接收数据，24位数据格式
- 第25个时钟脉冲用于设置增益和通道选择
*/

uint32_t Weight_Maopi;  // 皮重（空箱重量）校准值

/**
 * @brief       初始化HX711称重传感器
 * @param       无
 * @retval      无
 * @details     配置SCK和DOUT引脚，获取皮重校准值
 */
void HX711_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 使能GPIO时钟
    HX711_GPIO_CLK_ENABLE();

    // 配置SCK引脚为推挽输出
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

    delay_ms(100);  // 等待传感器稳定
    Weight_Maopi = HX711_Read();  // 获取皮重（空箱重量）
}

/**
 * @brief       读取HX711的24位ADC数据
 * @param       无
 * @retval      24位ADC转换结果（unsigned long）
 * @details     通过2线协议读取HX711的称重数据，返回原始AD值
 */
unsigned long HX711_Read(void)
{
    uint32_t data = 0;
    uint32_t i;

    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);  // 初始化时钟为低电平
  
    while(HAL_GPIO_ReadPin(HX711_DOUT_GPIO_Port, HX711_DOUT_Pin) == GPIO_PIN_SET);  // 等待DOUT变为低电平（传感器就绪）

    for(i = 0; i < 24; i++)  // 读取24位数据
    {
        HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);  // 上升沿
        data = data << 1;  // 数据左移
        HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);  // 下降沿
        if(HAL_GPIO_ReadPin(HX711_DOUT_GPIO_Port, HX711_DOUT_Pin) == GPIO_PIN_SET)  // 读取数据位
        {
            data++;
        }
    }

    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_SET);  // 第25个脉冲
    data = data ^ 0x800000;  // 转换为补码（处理负数）
    HAL_GPIO_WritePin(HX711_SCK_GPIO_Port, HX711_SCK_Pin, GPIO_PIN_RESET);

    return data;  // 返回24位ADC数据
}

/**
 * @brief       获取称重重量（单位：克）
 * @param       无
 * @retval      物体重量（int32_t，单位：克）
 * @details     读取当前重量并减去皮重，通过校准系数转换为实际重量
 */
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
    
    return objectWeight;  // 返回重量（克）
}
