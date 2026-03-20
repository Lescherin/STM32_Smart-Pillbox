#include "SYN6288.h"
#include <string.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include <cstdarg>
#include <stdio.h>  // vsprintf 函数声明在 stdio.h 中

extern UART_HandleTypeDef huart1;

//Music:选择背景音乐。0:无背景音乐，1~15：选择背景音乐
void SYN_FrameInfo(uint8_t Music, const char *HZdataFormat, ...) {
    char formattedData[100]; // 假设最大长度不会超过100字节（根据实际需要调整）
    va_list args;
    va_start(args, HZdataFormat);
    vsprintf(formattedData, HZdataFormat, args);
    va_end(args);

    // 计算formattedData的长度
    unsigned char HZ_Length = strlen(formattedData);

    // 初始化Frame_Info数组
    unsigned char Frame_Info[100]; // 确保足够大以存储整个帧
    unsigned char ecc = 0;
    unsigned int i = 0;

    // 构造帧头和数据长度
    Frame_Info[0] = 0xFD;
    Frame_Info[1] = 0x00;
    Frame_Info[2] = HZ_Length + 3;
    Frame_Info[3] = 0x01;
    Frame_Info[4] = 0x01 | Music << 4;

    // 计算校验码
    for (i = 0; i < 5; i++) {
        ecc ^= Frame_Info[i];
    }

    for (i = 0; i < HZ_Length; i++) {
        ecc ^= formattedData[i];
    }

    // 复制数据到Frame_Info数组，并添加校验码
    memcpy(&Frame_Info[5], formattedData, HZ_Length);
    Frame_Info[5 + HZ_Length] = ecc;

    // 发送帧信息
    HAL_UART_Transmit(&huart1, Frame_Info, (5 + HZ_Length + 1), HAL_MAX_DELAY);
}

