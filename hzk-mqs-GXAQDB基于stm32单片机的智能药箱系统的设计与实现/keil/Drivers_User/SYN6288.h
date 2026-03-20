#include <string.h>
#include <stdint.h>

/**
 * @brief SYN6288语音模块功能用途：
 * - 播放吃药提醒语音
 * - 播放系统状态提示
 * - 实现语音交互功能
 * 
 * @brief 引脚连接：
 * - 数据发送引脚（TX）：PA9
 * - 数据接收引脚（RX）：PA10
 * 
 * @brief 通信方式：
 * - UART串行通信（波特率：9600）
 * - 支持GB2312编码
 * - 支持多种语音合成参数设置
 */

/**
 * @brief 发送语音合成命令
 * @param Music 背景音乐选择：0-无背景音乐，1~15-选择背景音乐
 * @param HZdataFormat 格式化字符串
 * @param ... 可变参数列表
 */
void SYN_FrameInfo(uint8_t Music, const char *HZdataFormat, ...);

/**
 * @brief 设置语音合成参数
 * @param Info_data 参数数据
 */
void YS_SYN_Set(uint8_t *Info_data);

