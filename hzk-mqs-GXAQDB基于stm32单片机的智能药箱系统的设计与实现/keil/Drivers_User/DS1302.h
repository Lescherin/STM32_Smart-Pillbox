#ifndef __DS1302_H
#define __DS1302_H

#include "stm32f1xx_hal.h"

#define DS1302_CE_PORT    GPIOB
#define DS1302_CE_PIN     GPIO_PIN_7
#define DS1302_DATA_PORT  GPIOB
#define DS1302_DATA_PIN   GPIO_PIN_6
#define DS1302_SCLK_PORT  GPIOB
#define DS1302_SCLK_PIN   GPIO_PIN_5

#define CE_L    HAL_GPIO_WritePin(DS1302_CE_PORT, DS1302_CE_PIN, GPIO_PIN_RESET)
#define CE_H    HAL_GPIO_WritePin(DS1302_CE_PORT, DS1302_CE_PIN, GPIO_PIN_SET)

#define DATA_L  HAL_GPIO_WritePin(DS1302_DATA_PORT, DS1302_DATA_PIN, GPIO_PIN_RESET)
#define DATA_H  HAL_GPIO_WritePin(DS1302_DATA_PORT, DS1302_DATA_PIN, GPIO_PIN_SET)

#define SCLK_L  HAL_GPIO_WritePin(DS1302_SCLK_PORT, DS1302_SCLK_PIN, GPIO_PIN_RESET)
#define SCLK_H  HAL_GPIO_WritePin(DS1302_SCLK_PORT, DS1302_SCLK_PIN, GPIO_PIN_SET)

// DS1302时间结构体
typedef struct {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hour;
    uint8_t  minute;
    uint8_t  second;
    uint8_t  week;
} TIMEData;

typedef struct {
    uint8_t  hour_kai;
    uint8_t  minute_kai;
    uint8_t  hour_guan;
    uint8_t  minute_guan;
    uint8_t  kai;
    uint8_t  guan;
} TIMERAM;

extern TIMERAM TimeRAM;
extern TIMEData TimeData;

void DS1302_GPIO_Init(void);
void DS1302_write_onebyte(uint8_t data);
void DS1302_wirte_rig(uint8_t address, uint8_t data);
uint8_t DS1302_read_rig(uint8_t address);
void DS1302_Init(const TIMEData  *time);  // 初始化DS1302时间
void DS1302_DATAOUT_init(void);
void DS1302_DATAINPUT_init(void);
void DS1302_read_time(void);
void DS1302_read_realTime(void); // 读取DS1302实时时间(并转换为时间结构体)
void DS1302_wirteRAM(void);
void DS1302_readRAM(void);

#endif


