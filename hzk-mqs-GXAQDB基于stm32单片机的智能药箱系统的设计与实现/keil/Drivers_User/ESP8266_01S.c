#include "ESP8266_01S.h"
#include <string.h>
#include "stdarg.h"	
#include "usart.h"	
#include "delay.h"	
#include "oled.h"	


#include "FreeRTOS.h"
#include "queue.h"

/* 收发Buffer */
char Usart2_RxBuff[USART2_RXBUFF_SIZE];
char Usart2_TxBuff[USART2_TXBUFF_SIZE];
/* 接收到的数据长度计数器 */
uint16_t Usart2_RxCounter;	

/* 消息队列句柄 */
extern QueueHandle_t Wifi_Message_Queue;		

/* 定义uart2发送数据长度变量 */
static unsigned int length;

/* MQTT协议封装部分 */
const uint8_t parket_connetAck[] = {0x20,0x02,0x00,0x00};
const uint8_t parket_disconnet[] = {0xe0,0x00};
const uint8_t parket_heart[] = {0xc0,0x00};
const uint8_t parket_heart_reply[] = {0xc0,0x00};
const uint8_t parket_subAck[] = {0x90,0x03};
const uint8_t packet_content[] = {0x10,0x1D,0x00,0x04,0x4D,0x51,0x54,0x54,0x04,0xC2,0x00,0x3C,0x00,0x05,0x73,0x74,0x6D,0x33,0x32,0x00,0x04,0x72,0x6F,0x6F,0x74,0x00,0x04,0x72,0x6F,0x6F,0x74};



/* 重定义uart2发送方法 */
void uart2_printf(char* fmt,...) 
{  
	unsigned int i;
	
	va_list ap;
	va_start(ap,fmt);
	vsprintf(Usart2_TxBuff,fmt,ap);
	va_end(ap);	
	
	length=strlen((const char*)Usart2_TxBuff);
	while((USART2->SR&0X40)==0);
	for(i = 0;i < length;i ++)
	{			
		USART2->DR = Usart2_TxBuff[i];
		while((USART2->SR&0X40)==0);	
	}	
}

/* 定义uart2发送数据是否成功判断方法 */
char WiFi_SendCmd(char *cmd, int timeout)
{
    Usart2_RxCounter = 0;
    memset(Usart2_RxBuff, 0, USART2_RXBUFF_SIZE);
    
    // 发送命令
    uart2_printf("%s\r\n", cmd);
    
    // 等待响应
    while(timeout--) {
        delay_ms(100);
        
        // 检查各种可能的响应
        if(strstr(Usart2_RxBuff, "OK") != NULL || 
           strstr(Usart2_RxBuff, "SEND OK") != NULL ||
           strstr(Usart2_RxBuff, "WIFI GOT IP") != NULL) {
            delay_ms(100);
            return 0; // 成功
        }
        
        if(strstr(Usart2_RxBuff, "ERROR") != NULL || 
           strstr(Usart2_RxBuff, "FAIL") != NULL) {
            return 1; // 错误
        }
    }
    
    // 超时处理
    OLED_Clear();
    OLED_Printf(0, 6, 8, "Timeout!");
    return 1;
}


void ESP_RST(void)
{
	ESR_RST_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	GPIO_InitStruct.Pin  = ESP_RST_PIN;
	HAL_GPIO_Init(ESP_RST_PORT, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(ESP_RST_PORT, ESP_RST_PIN, GPIO_PIN_RESET);
	delay_ms(2000);
	HAL_GPIO_WritePin(ESP_RST_PORT, ESP_RST_PIN, GPIO_PIN_SET);
}
/* 定义WiFi复位方法 */
char WiFi_Reset(int timeout)
{
	Usart2_RxCounter=0;													//uart2接收数据长度                  
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE); //清空uart2接收缓冲区 
//	uart2_printf("AT+RST\r\n");							//向ESP发送复位指令
	//采用硬件的软复位 拉低RESET引脚
	ESP_RST();
	while(timeout--){                               //等待超时时间到0
		delay_ms(100);                              //延时100ms
		if(strstr(Usart2_RxBuff,"OK")!= 0)             //如果接收到ready表示复位成功
		{
			delay_ms(2000);
			break;       						    //主动跳出while循环
		}
	}
	if(timeout<=0)return 1;                         //如果timeout<=0，说明超时时间到了，也没能收到ready，返回1
	else return 0;	         				    //反之，表示正确，说明收到ready，通过break主动跳出while
}
/* 发送数据给服务器 */
char WiFi_Send(char *sendbuf)
{
	   memset(Usart2_RxBuff, 0, USART2_RXBUFF_SIZE);
    //检查输入数据是否有效
    if(sendbuf != NULL)
    {
        uart2_printf("%s",sendbuf);              //通过串口发送数据到ESP8266
        delay_ms(50);
        return 0;
    }
    else
    {
        return 1;
    }
}

/*WIFI连接服务器*/
char WiFi_Connect_Server(int timeout)
{	
	Usart2_RxCounter=0;                               //WiFi接收数据量变量清零                        
	memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);         //清空WiFi接收缓冲区   
	uart2_printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",SERVER_IP_ADDRESS,SERVER_IP_PORT);//发送连接服务器指令
	while(timeout--){                               //等待超时与否
		delay_ms(100);                              //延时100ms	
		if(strstr(Usart2_RxBuff ,"CONNECT"))          //如果接受到CONNECT表示连接成功
			break;                                  //跳出while循环
		if(strstr(Usart2_RxBuff ,"CLOSED"))           //如果接受到CLOSED表示服务器未开启
			return 1;                               //服务器未开启返回1
		if(strstr(Usart2_RxBuff ,"ALREADY CONNECTED"))//如果接受到ALREADY CONNECTED已经建立连接
			return 2;                               //已经建立连接返回2
	}
	
	if(timeout<=0)return 3;                   //超时错误，返回3
	else                                      //连接成功，准备进入透传
	{
		uart2_printf("AT+CIPMODE=1\r\n"); 
		delay_ms(2000);
		Usart2_RxCounter=0;    
		memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);         //清空WiFi接收缓冲区     
		uart2_printf("AT+CIPSEND\r\n");                  //发送进入透传指令
		while(timeout--){                               //等待超时与否
			delay_ms(2000);                              //延时100ms	
			if(strstr(Usart2_RxBuff,">"))   //如果成立表示进入透传成功
				break;                          //跳出while循环
			Usart2_RxCounter=0;                               //WiFi接收数据量变量清零                        
			memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE);         //清空WiFi接收缓冲区     
			uart2_printf("AT+CIPSEND\r\n");                  //发送进入透传指令
		}
		if(timeout<=0)return 4;                 //透传超时错误，返回4	
	}
	return 0;	                                //成功返回0	
}

//MQTT发送数据
void MQTT_SendBuf(uint8_t *buf,uint16_t len)
{
    int i=0;  
    while((USART2->SR&0X40)==0);
	for(i = 0;i < len;i ++)
	{			
		//USART2->DR = Usart2_TxBuff[i];
        USART2->DR = buf[i];
		while((USART2->SR&0X40)==0);	
	}	
}	



volatile uint16_t MQTT_TxLen;

//MQTT连接服务器的打包函数
uint8_t MQTT_Connect(char *ClientID,char *Username,char *Password)
{
	int ClientIDLen = strlen(ClientID);
	int UsernameLen = strlen(Username);
	int PasswordLen = strlen(Password);
	int DataLen;
 	MQTT_TxLen=0;
	//可变报头+Payload  每个字段包含两个字节的长度标识
    DataLen = 10 + (ClientIDLen+2) + (UsernameLen+2) + (PasswordLen+2);

    //固定报头
    //控制报文类型
    Usart2_TxBuff[MQTT_TxLen++] = 0x10;		//MQTT Message Type CONNECT
	//剩余长度(不包括固定头部)
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		Usart2_TxBuff[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );
    	
	//可变报头
	//协议名
	Usart2_TxBuff[MQTT_TxLen++] = 0;        		// Protocol Name Length MSB    
	Usart2_TxBuff[MQTT_TxLen++] = 4;        		// Protocol Name Length LSB    
	Usart2_TxBuff[MQTT_TxLen++] = 'M';        	// ASCII Code for M    
	Usart2_TxBuff[MQTT_TxLen++] = 'Q';        	// ASCII Code for Q    
	Usart2_TxBuff[MQTT_TxLen++] = 'T';        	// ASCII Code for T    
	Usart2_TxBuff[MQTT_TxLen++] = 'T';        	// ASCII Code for T    
	//协议级别
	Usart2_TxBuff[MQTT_TxLen++] = 4;        		// MQTT Protocol version = 4    
	//连接标志
	Usart2_TxBuff[MQTT_TxLen++] = 0xc2;        	// conn flags 
	Usart2_TxBuff[MQTT_TxLen++] = 0;        	// Keep-alive Time Length MSB    
	Usart2_TxBuff[MQTT_TxLen++] = 60;        	// Keep-alive Time Length LSB  60S心跳包  

	Usart2_TxBuff[MQTT_TxLen++] = BYTE1(ClientIDLen);// Client ID length MSB    
	Usart2_TxBuff[MQTT_TxLen++] = BYTE0(ClientIDLen);// Client ID length LSB  	
	memcpy(&Usart2_TxBuff[MQTT_TxLen],ClientID,ClientIDLen);
	MQTT_TxLen += ClientIDLen;
	
	if(UsernameLen > 0)
	{   
		Usart2_TxBuff[MQTT_TxLen++] = BYTE1(UsernameLen);		//username length MSB    
		Usart2_TxBuff[MQTT_TxLen++] = BYTE0(UsernameLen);    	//username length LSB    
		memcpy(&Usart2_TxBuff[MQTT_TxLen],Username,UsernameLen);
		MQTT_TxLen += UsernameLen;
	}
	
	if(PasswordLen > 0)
	{    
		Usart2_TxBuff[MQTT_TxLen++] = BYTE1(PasswordLen);		//password length MSB    
		Usart2_TxBuff[MQTT_TxLen++] = BYTE0(PasswordLen);    	//password length LSB  
		memcpy(&Usart2_TxBuff[MQTT_TxLen],Password,PasswordLen);
		MQTT_TxLen += PasswordLen; 
	}    
	
	delay_ms(1000);
	Usart2_RxCounter=0;                       //WiFi接收数据量变量清零                        
  memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE); //清空WiFi接收缓冲区
	MQTT_SendBuf((uint8_t*)Usart2_TxBuff,MQTT_TxLen);
	delay_ms(1000);
	if(Usart2_RxBuff[0]==0x20&&Usart2_RxBuff[1]==0x02&&Usart2_RxBuff[2]==0x00&&Usart2_RxBuff[3]==0x00)
	if(Usart2_RxBuff[0]==parket_connetAck[0] && Usart2_RxBuff[1]==parket_connetAck[1]) //连接成功			   
	{
		return 1;//连接成功
	}
	return 0;
}

//MQTT订阅/取消订阅数据打包函数
//topic       主题 
//qos         消息等级 
//whether     订阅/取消订阅请求包
uint8_t MQTT_SubscribeTopic(char *topic,uint8_t qos,uint8_t whether)
{    
	int topiclen;
	int DataLen;
//    int i=0;
    
	MQTT_TxLen=0;
    topiclen = strlen(topic);
    DataLen = 2 + (topiclen+2) + (whether?1:0);//可变报头的长度（2字节）加上有效载荷的长度
    
    //固定报头
	//控制报文类型
	if(whether) Usart2_TxBuff[MQTT_TxLen++] = 0x82; //消息类型和标志订阅
	else	Usart2_TxBuff[MQTT_TxLen++] = 0xA2;    //取消订阅

	//剩余长度
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		Usart2_TxBuff[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );	
	
	//可变报头
	Usart2_TxBuff[MQTT_TxLen++] = 0;				//消息标识符 MSB
	Usart2_TxBuff[MQTT_TxLen++] = 0x01;           //消息标识符 LSB
	//有效载荷
	Usart2_TxBuff[MQTT_TxLen++] = BYTE1(topiclen);//主题长度 MSB
	Usart2_TxBuff[MQTT_TxLen++] = BYTE0(topiclen);//主题长度 LSB   
	memcpy(&Usart2_TxBuff[MQTT_TxLen],topic,topiclen);
	MQTT_TxLen += topiclen;

	if(whether)
	{
		Usart2_TxBuff[MQTT_TxLen++] = qos;//QoS级别
	}
		delay_ms(1000);
		Usart2_RxCounter=0;                       //WiFi接收数据量变量清零                        
		memset(Usart2_RxBuff,0,USART2_RXBUFF_SIZE); //清空WiFi接收缓冲区
		MQTT_SendBuf((uint8_t *)Usart2_TxBuff,MQTT_TxLen);
		delay_ms(1000);
		if(Usart2_RxBuff[0]==parket_subAck[0] && Usart2_RxBuff[1]==parket_subAck[1]) //订阅成功			   
		{
			return 1;//订阅成功
		}
	return 0;
}

//MQTT发布数据打包函数
//topic   主题 
//message 消息
//qos     消息等级 
uint8_t MQTT_PublishData(char *message, uint8_t qos)
{  
	int topicLength = strlen(MQTT_TOPIC);    
	int messageLength = strlen(message);     
	static uint16_t id=0;
	int DataLen;
//	memset(Usart2_TxBuff, 0, sizeof(Usart2_TxBuff));  // 清空缓冲区

	MQTT_TxLen=0;
	//有效载荷的长度这样计算：用固定报头中的剩余长度字段的值减去可变报头的长度
	//QOS为0时没有标识符
	//数据长度             主题名   报文标识符   有效载荷
	if(qos)	DataLen = (2+topicLength) + 2 + messageLength;       
	else	DataLen = (2+topicLength) + messageLength;   

    //固定报头
	//控制报文类型
	Usart2_TxBuff[MQTT_TxLen++] = 0x30;    // MQTT Message Type PUBLISH  

	//剩余长度
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		Usart2_TxBuff[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );	
	
	Usart2_TxBuff[MQTT_TxLen++] = BYTE1(topicLength);//主题长度MSB
	Usart2_TxBuff[MQTT_TxLen++] = BYTE0(topicLength);//主题长度LSB 
	memcpy(&Usart2_TxBuff[MQTT_TxLen],MQTT_TOPIC,topicLength);//拷贝主题
	MQTT_TxLen += topicLength;
        
	//报文标识符
	if(qos)
	{
			Usart2_TxBuff[MQTT_TxLen++] = BYTE1(id);
			Usart2_TxBuff[MQTT_TxLen++] = BYTE0(id);
			id++;
	}
	memcpy(&Usart2_TxBuff[MQTT_TxLen],message,messageLength);
    MQTT_TxLen += messageLength;
        
	MQTT_SendBuf((uint8_t *)Usart2_TxBuff,MQTT_TxLen);
  return MQTT_TxLen;
}

uint8_t WiFi_init(void)
{  
    /* 测试AT指令 */
    if(WiFi_Reset(50)){
        
    };
    if(WiFi_SendCmd("AT+CIPCLOSE",50)){

    };
    /* 复位ESP8266 */
    if(WiFi_Reset(50)){

    };
    /* 进入Station模式 */
    if(WiFi_SendCmd("AT+CWMODE=1",50)){

    };

    /* 取消自动连接 */
    if(WiFi_SendCmd("AT+CWAUTOCONN=1",50)){

    };

    /* 启用DHCP */
    if(WiFi_SendCmd("AT+CWDHCP=1,1",50)){

    };

    /* 连接WIFI信号 */
    if(WiFi_SendCmd(AT_CMD_JOIN_AP("AT+CWJAP", WIFI_SSID, WIFI_PASS), 100)){

    };


    if(WiFi_Connect_Server(50)){

    }

    if(MQTT_Connect(MQTT_TOPIC,MQTT_USER_NAME,MQTT_USER_PASS)!=1){

    }

    if(MQTT_SubscribeTopic(ORDER_MQTT_TOPIC,0,1) != 1)
    {

    }


    return 0;
    }



