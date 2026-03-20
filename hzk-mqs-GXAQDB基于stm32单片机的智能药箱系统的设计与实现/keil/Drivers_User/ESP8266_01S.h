#ifndef __ESP8266_H
#define __ESP8266_H
#include <stdio.h>
#include "sys.h" 

/**
 * @brief ESP8266 WiFi模块功能用途：
 * - 实现药箱与云端的MQTT通信
 * - 上传传感器数据到云端
 * - 接收云端控制指令
 * - 实现远程监控和控制功能
 * 
 * @brief 引脚连接：
 * - 数据发送引脚（TX）：PA2
 * - 数据接收引脚（RX）：PA3
 * - 复位引脚（RST）：PA15
 * 
 * @brief 通信方式：
 * - UART串行通信（波特率：115200）
 * - MQTT协议实现云端通信
 * - 支持TCP/IP协议栈
 */

#define	USART2_RXBUFF_SIZE	200 	//USART2接收缓冲区大小
#define	USART2_TXBUFF_SIZE	200		//发送缓冲区大小



/****************************/

/* WIFI通信配置 */
#define WIFI_SSID "lesch"
#define WIFI_PASS "Misono_508"

/* MQTT订阅配置 */
/**
	名称:MQTT_TOPIC 数据发布主题
	作用:单片机→服务器 方向的消息通道 sendTask
	功能:单片机将传感器数据（温度、湿度、重量、TCRT5000等）通过这个主题名称发送到MQTT服务
	
	名称:ORDER_MQTT_TOPIC 控制命令主题
	作用：服务器→单片机 方向的消息通道 recieveTask
	功能:在MQTTfx中向此主题发布消息，可以 远程控制智能药箱 （如控制蜂鸣器、舵机等）
*/
#define MQTT_TOPIC "hznyxxt2025114"
#define ORDER_MQTT_TOPIC "hznyxxt2025114S_ctrl"

/* MQTT用户配置 */
#define MQTT_USER_NAME "root"
#define MQTT_USER_PASS "root"
#define CLIENT_ID "Test"

/* MQTT链接地址 */
#define SERVER_IP_ADDRESS "120.27.235.176"
#define SERVER_IP_PORT 1883

/****************************/


// 使用宏拼接生成完整命令
#define AT_CMD_JOIN_AP(cmd, ssid, pass)  cmd "=\""ssid"\",\""pass"\""

// 使用宏拼接生成完整命令
#define MQTT_USER_CONFIG(cmd, user, pass , cid)  cmd "=0,1,\""user"\",\""pass"\",\""cid"\",0,0,\"\""

// 使用宏拼接生成完整命令
#define MQTT_CONNECT(cmd, addr, port)  cmd "=0,\"" addr "\"," port ",1"

// 使用宏拼接生成完整命令
#define MQTT_CONN_TOPIC(cmd, topic)  cmd "=0,\""topic"\",0"

extern char Usart2_RxBuff[USART2_RXBUFF_SIZE];
extern char Usart2_TxBuff[USART2_TXBUFF_SIZE];
extern uint16_t Usart2_RxCounter;

/* MQTT协议封装部分 */
extern const uint8_t parket_connetAck[];
extern const uint8_t parket_disconnet[];
extern const uint8_t parket_heart[];
extern const uint8_t parket_heart_reply[];
extern const uint8_t parket_subAck[];
extern const uint8_t packet_content[];
	
void USART2_sendonechar(uint8_t data);
void USART2_sendstring(uint8_t *data,uint16_t length);

#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))


/****************************/
/*硬件引脚RST*/

#define ESP_RST_PIN  GPIO_PIN_15
#define ESP_RST_PORT GPIOA
#define ESR_RST_CLK_ENABLE()          do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* PG口时钟使能 */

/* 声明uart2串口输出函数 */
void uart2_printf(char*,...);

/* 声明uart2串口输出函数 */
uint8_t WiFi_init(void);

/* 定义uart2发送数据是否成功判断方法 */
char WiFi_SendCmd(char *cmd, int timeout);

/* 定义WiFi复位方法 */
char WiFi_Reset(int timeout);

/* 定义WiFi发送方法 */
char WiFi_Send(char *sendbuf);

/* 定义链接服务器方法 */
char WiFi_Connect_Server(int timeout);

/* MQTT发送数据 */
void MQTT_SendBuf(uint8_t *buf,uint16_t len);

/* MQTT连接服务器的打包函数 */
uint8_t MQTT_Connect(char *ClientID,char *Username,char *Password);

/* MQTT订阅/取消订阅数据打包函数 */
uint8_t MQTT_SubscribeTopic(char *topic,uint8_t qos,uint8_t whether);

/* MQTT发布数据打包函数 */
uint8_t MQTT_PublishData(char *message, uint8_t qos);
#endif
