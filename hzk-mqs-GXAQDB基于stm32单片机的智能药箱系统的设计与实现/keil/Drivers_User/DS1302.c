#include "DS1302.h"
#include "delay.h"


/**
DS1302实时时钟
- 提供实时时间和日期信息
- 可设置定时开关药箱的功能
- 记录药品使用时间戳
引脚连接 ：

- 时钟引脚（SCLK）： PB5
- 使能引脚（CE/RST）： PB7
- 数据引脚（I/O）： PB6
通信方式 ：

- 3线串行接口（时钟+使能+数据）
- 数据引脚为双向I/O，需要动态切换输入/输出模式
- 先发送地址，再发送/接收数据
*/

TIMERAM TimeRAM;
TIMEData TimeData;
uint8_t read_time[7];

// DS1302 GPIO初始化
void DS1302_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
		
		
    GPIO_InitStruct.Pin = DS1302_SCLK_PIN | DS1302_CE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DS1302_SCLK_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(DS1302_SCLK_PORT, DS1302_SCLK_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DS1302_CE_PORT, DS1302_CE_PIN, GPIO_PIN_RESET);
}

// 设置数据引脚为输出模式
void DS1302_DATAOUT_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DS1302_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DS1302_DATA_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(DS1302_DATA_PORT, DS1302_DATA_PIN, GPIO_PIN_RESET);
}

// 设置数据引脚为输入模式
void DS1302_DATAINPUT_init() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DS1302_DATA_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(DS1302_DATA_PORT, &GPIO_InitStruct);
}

// 向DS1302写入一个字节
void DS1302_write_onebyte(uint8_t data) {
    uint8_t count = 0;
    SCLK_L;
    DS1302_DATAOUT_init();

    for (count = 0; count < 8; count++) {
        SCLK_L;
        if (data & 0x01) {
            DATA_H;
        } else {
            DATA_L;
        }
        SCLK_H;
        data >>= 1;
    }
}

// 向DS1302寄存器写入数据
void DS1302_wirte_rig(uint8_t address, uint8_t data) {
    uint8_t temp1 = address;
    uint8_t temp2 = data;

    CE_L;
    SCLK_L;
    delay_us(10);
    CE_H;
    delay_us(10);

    DS1302_write_onebyte(temp1);
    DS1302_write_onebyte(temp2);

    CE_L;
    SCLK_L;
    delay_us(10);
}

// 从DS1302寄存器读取数据
uint8_t DS1302_read_rig(uint8_t address) {
    uint8_t temp3 = address;
    uint8_t count = 0;
    uint8_t return_data = 0x00;

    CE_L;
    SCLK_L;
    delay_us(10);
    CE_H;
    delay_us(10);

    DS1302_write_onebyte(temp3);
    DS1302_DATAINPUT_init();
    delay_us(10);

    for (count = 0; count < 8; count++) {
        delay_us(10);
        return_data >>= 1;
        SCLK_H;
        delay_us(10);
        SCLK_L;
        delay_us(10);
        if (HAL_GPIO_ReadPin(DS1302_DATA_PORT, DS1302_DATA_PIN)) {
            return_data = return_data | 0x80;
        }
    }

    delay_us(10);
    CE_L;
    DATA_L;
    return return_data;
}

// 初始化DS1302时钟
void DS1302_Init(const TIMEData *time) 
	{
		unsigned char  sec_bcd = ((time->second / 10) << 4) | (time->second % 10);
    unsigned char min_bcd = ((time->minute / 10) << 4) | (time->minute % 10);
    unsigned char hour_bcd = ((time->hour / 10) << 4) | (time->hour % 10);
    unsigned char day_bcd = ((time->day / 10) << 4) | (time->day % 10);
    unsigned char month_bcd = ((time->month / 10) << 4) | (time->month % 10);
    unsigned char year_bcd = ((time->year - 2000) / 10) << 4 | ((time->year - 2000) % 10);

    DS1302_wirte_rig(0x8e, 0x00); // 关闭写保护
    DS1302_wirte_rig(0x80, sec_bcd); // seconds
    DS1302_wirte_rig(0x82, min_bcd); // minutes
    DS1302_wirte_rig(0x84, hour_bcd); // hours
    DS1302_wirte_rig(0x86, day_bcd); // date
    DS1302_wirte_rig(0x88, month_bcd); // months
    DS1302_wirte_rig(0x8a, time->week); // days
    DS1302_wirte_rig(0x8c, year_bcd); // year
    DS1302_wirte_rig(0x8e, 0x80); // 开启写保护
}

// 读取DS1302实时时间
void DS1302_read_time(void) {
    read_time[0] = DS1302_read_rig(0x81);
    read_time[1] = DS1302_read_rig(0x83);
    read_time[2] = DS1302_read_rig(0x85);
    read_time[3] = DS1302_read_rig(0x87);
    read_time[4] = DS1302_read_rig(0x89);
    read_time[5] = DS1302_read_rig(0x8B);
    read_time[6] = DS1302_read_rig(0x8D);
}

// 转换为TimeData时间结构体
void DS1302_read_realTime(void) {
    DS1302_read_time();
    TimeData.second = (read_time[0] >> 4) * 10 + (read_time[0] & 0x0f);
    TimeData.minute = (read_time[1] >> 4) * 10 + (read_time[1] & 0x0f);
    TimeData.hour = (read_time[2] >> 4) * 10 + (read_time[2] & 0x0f);
    TimeData.day = (read_time[3] >> 4) * 10 + (read_time[3] & 0x0f);
    TimeData.month = (read_time[4] >> 4) * 10 + (read_time[4] & 0x0f);
    TimeData.week = read_time[5];
    TimeData.year = (read_time[6] >> 4) * 10 + (read_time[6] & 0x0f) + 2000;
}

void DS1302_wirteRAM(void) {
    DS1302_wirte_rig(0x8e, 0x00);
    DS1302_wirte_rig(0xC0, TimeRAM.hour_kai);
    DS1302_wirte_rig(0xC2, TimeRAM.minute_kai);
    DS1302_wirte_rig(0xC4, TimeRAM.hour_guan);
    DS1302_wirte_rig(0xC6, TimeRAM.minute_guan);
    DS1302_wirte_rig(0xC8, TimeRAM.kai);
    DS1302_wirte_rig(0xCA, TimeRAM.guan);
    DS1302_wirte_rig(0x8e, 0x80);
}

void DS1302_readRAM(void) {
    TimeRAM.hour_kai = DS1302_read_rig(0xC1);
    TimeRAM.minute_kai = DS1302_read_rig(0xC3);
    TimeRAM.hour_guan = DS1302_read_rig(0xC5);
    TimeRAM.minute_guan = DS1302_read_rig(0xC7);
    TimeRAM.kai = DS1302_read_rig(0xC9);
    TimeRAM.guan = DS1302_read_rig(0xCB);
}

