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

/**
 * @brief       SYN6288语音合成芯片帧信息发送函数
 * @param       Music: 背景音乐选择 (0:无背景音乐, 1~15:选择背景音乐1~15)
 * @param       HZdataFormat: 中文文本格式化字符串（支持格式化参数）
 * @param       ...: 可变参数列表（对应格式化字符串中的占位符）
 * @retval      无
 * @details     该函数用于向SYN6288语音合成芯片发送语音合成指令帧
 *              实现功能：将文本内容通过UART串口传输给SYN6288芯片进行语音合成播放
 * 
 * 通信协议说明：
 * - 串口接口：UART1 (USART1)
 * - 帧格式：FD 00 Length 01 01|Music ECC Data... ECC
 * - Length字段：数据长度 = 中文字符长度 + 3（控制字节+音乐控制字节+校验字节）
 * - 校验方式：异或校验（XOR），从帧头开始到数据末尾
 * - 数据格式：GB2312编码的中文文本
 * 
 * 应用场景：
 * - 智能药箱语音播报药品信息
 * - 提醒用户取药时间
 * - 播放药品使用说明
 * 
 * 示例：
 *   SYN_FrameInfo(0, "请按时服药");  // 无背景音乐，播报"请按时服药"
 *   SYN_FrameInfo(1, "第%d号药盒已准备好", 3);  // 背景音乐1，播报"第3号药盒已准备好"
 */
void SYN_FrameInfo(uint8_t Music, const char *HZdataFormat, ...) {
    char formattedData[100]; // 假设最大长度不会超过100字节（根据实际需要调整）
    va_list args;
    va_start(args, HZdataFormat);
    vsprintf(formattedData, HZdataFormat, args);
    va_end(args);

    // 计算formattedData的长度（中文字符长度）
    unsigned char HZ_Length = strlen(formattedData);

    // 初始化Frame_Info数组（确保足够大以存储整个帧）
    unsigned char Frame_Info[100];
    unsigned char ecc = 0;
    unsigned int i = 0;

    // 构造帧头和数据长度
    Frame_Info[0] = 0xFD;           // 帧头标识（固定值0xFD）
    Frame_Info[1] = 0x00;           // 命令字高字节（固定值0x00）
    Frame_Info[2] = HZ_Length + 3;  // 数据长度 = 中文字符长度 + 3（控制字节+音乐控制字节+校验字节）
    Frame_Info[3] = 0x01;           // 命令字低字节（固定值0x01，表示文本合成命令）
    Frame_Info[4] = 0x01 | Music << 4;  // 音乐控制字节（低4位固定为01，高4位为背景音乐选择）

    // 计算校验码（异或校验：从帧头到数据末尾所有字节的异或结果）
    for (i = 0; i < 5; i++) {
        ecc ^= Frame_Info[i];
    }

    for (i = 0; i < HZ_Length; i++) {
        ecc ^= formattedData[i];
    }

    // 复制数据到Frame_Info数组，并添加校验码
    memcpy(&Frame_Info[5], formattedData, HZ_Length);
    Frame_Info[5 + HZ_Length] = ecc;  // 校验码放在数据末尾

    // 发送帧信息（通过UART1发送完整帧）
    HAL_UART_Transmit(&huart1, Frame_Info, (5 + HZ_Length + 1), HAL_MAX_DELAY);
}

