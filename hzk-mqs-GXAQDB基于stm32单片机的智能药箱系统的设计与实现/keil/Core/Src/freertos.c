#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "string.h"
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128,
  .priority = (osPriority_t)(osPriorityNormal),
};
void StartDefaultTask(void *argument);
void MX_FREERTOS_Init(void); 

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "oled.h"
#include "ESP8266_01S.h"
#include "GPIO_OUT.h"
#include "adc.h"
#include "GPIO_IN.H"
#include "sg90.h"
#include "dht11.h"
#include "syn6288.h"
#include "HX711.h"
#include "DS1302.h"
#include "key.h"

// 初始化DS1302时间
/*
void DS1302_Init(void) {
    DS1302_GPIO_Init();
    DS1302_DATAOUT_init();
    DS1302_wirte_rig(0x8e, 0x00); // 关闭写保护
    DS1302_wirte_rig(0x8a, TIMEDATA_Init.week); // days
    DS1302_wirte_rig(0x8c, TIMEDATA_Init.year_bcd); // year
    DS1302_wirte_rig(0x8e, 0x80); // 开启写保护
}
*/



 //全局变量

 //在adc.c对应uint32_t adc_get_result(uint8_t channel);   /* 获取ADC转换结果 */
uint32_t TCRT5000;

//在BEEP.h对应void BEEP_ON(void);   /* 蜂鸣器-on */
uint32_t BEEP;

// 在SG90.h对应sg90_set_angle(uint16_t angle);   /* 设置角度 */
uint32_t SG90;

//在dht11.h对应dht11_read_data(struct dht11_Struct *data);   /* 读取温湿度 */
struct dht11_Struct DHT11_Data;

// 在HX711.c对应float Get_Weight(void);   /* 读取重量 */
float Weight;



/**
	* @说明:传感器数据采集任务
	* @功能:
		*1. 读取传感器各项数据（温湿度、重量、红外传感器等）
		*2. 存储数据到全局变量
        *3. 读取实时时间
        创建临界区:时钟的时间不允许被中断或其他任务抢占,确保数据访问的原子性和一致性 。
	* @传递数据格式:无
*/
void Sensor_Task(void *argument){

    while(1){
        //读取传感器各项数据
        dht11_read_data(&DHT11_Data);               //温湿度
        TCRT5000 = adc_get_result(ADC_CHANNEL_1);   //红外传感器
        Weight= Get_Weight();                       //重量

        // 读取实时时间（建立临界区,read_realTime()在临界区内执行，保护共享资源）
        taskENTER_CRITICAL();
        DS1302_read_realTime();                     // 读取DS1302实时时间
        taskEXIT_CRITICAL();
         
        vTaskDelay(1000); 
    }
}

/**
	* @说明:OLED显示任务
	* @功能:
		*1. 显示传感器数据（温湿度、重量、红外传感器等）
		*2. 显示时间
	* @传递数据格式:无
*/
void Oled_Show(void *argument)
{
    while(1) {
				OLED_Clear();
			
        OLED_Printf(0,0,6,"T:%d   H:%d     ",DHT11_Data.temp	,DHT11_Data.humi);
        OLED_Printf(0,12,6,"Weight:%.1fg      ",Weight);
        OLED_Printf(0,24,6,"TCRT5000:%d     ",TCRT5000);
        
        OLED_ShowNum(0,36,TimeData.hour,2,6);
        OLED_ShowString(20, 36, ":", 6);
        OLED_ShowNum(30,36,TimeData.minute,2,6);
        OLED_ShowString(50, 36, ":", 6);
        OLED_ShowNum(60,36,TimeData.second,2,6);

        OLED_Update();
        HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
        vTaskDelay(500);
    }
}

/**
	* @说明:MQTT命令接收与处理
	* @功能:
		*1. 传感器数据采集（温湿度、重量、红外传感器等）
		*2. 数据格式化为JSON字符串
		*3. 通过MQTT协议发布到指定主题
	* @传递数据格式:json
*/
void ESP_SendTask(void *argument)
{
    char josnstr[200];          /* 即将发送的json字符串 */
    char tmp[50];               /* 格式化数据临时存储 */
    while(1)
    {  
        memset(josnstr,0,200);      /* 清空json ,追加json头 ,格式化主题唯一标识 */
        sprintf(tmp,"{\"Topic name\":\"");
        strcat(josnstr,tmp);
        
        sprintf(tmp,"\"");
        strcat(josnstr,tmp);
        
        //数据主题
        sprintf(tmp,MQTT_TOPIC);  
        strcat(josnstr,tmp);
        
        sprintf(tmp,"\"");
        strcat(josnstr,tmp);
        
        // 温湿度数据
        sprintf(tmp,",\"temp\":%d",DHT11_Data.temp);
        strcat(josnstr,tmp);
        
        sprintf(tmp,",\"humi\":%d",DHT11_Data.humi);
        strcat(josnstr,tmp);
        
        // 蜂鸣器状态
        sprintf(tmp,",\"beep\":%d",BEEP);
        strcat(josnstr,tmp);
        
        // 舵机状态
        sprintf(tmp,",\"sg90\":%d",SG90);
        strcat(josnstr,tmp);
        
        // 红外传感器状态
        sprintf(tmp,",\"tcrt5000\":%d",TCRT5000);
        strcat(josnstr,tmp);
        
        // 重量数据
        sprintf(tmp,",\"weight\":%.1f",Weight);
        strcat(josnstr,tmp);
        
        strcat(josnstr,"}");
				
				// 通过MQTT发送数据到云端
        MQTT_PublishData(josnstr,0);
        vTaskDelay(1000);
    }
}

/**
	* @说明:MQTT命令接收与处理任务
	* @功能:
		*1. 监听MQTT订阅的命令主题
		*2. 解析接收到的控制命令
		*3. 执行相应的操作（蜂鸣器开关、舵机控制等）
    * @命令说明:
	beep:on     ：打开蜂鸣器
	beep:off    ：关闭蜂鸣器
	sg90:on ：控制舵机旋转90度（出药）
	sg90:off    ：控制舵机旋转0度（复位）
	weight:on   ：重量不足提醒
	weight:off  ：关闭重量提醒
	* @传递数据格式:json
*/
void ESP_ReceiveTask(void *argument)
{
    while(1)
    {  
        if(Usart2_RxBuff[0] != 0x00){
            if(strstr(Usart2_RxBuff,"beep:on")){
                BEEP=1;
                BEEP_OUT(0);
                Usart2_RxCounter = 0;
                memset(Usart2_RxBuff, 0, USART2_RXBUFF_SIZE);
            }
            
            if(strstr(Usart2_RxBuff,"beep:off")){
                BEEP=0;
                BEEP_OUT(1);
                Usart2_RxCounter = 0;
                memset(Usart2_RxBuff, 0, USART2_RXBUFF_SIZE);
            }

            if(strstr(Usart2_RxBuff,"sg90:on")){
                SG90=1;
                set_servo_angle(90);
                BEEP=1;
                BEEP_OUT(0);
//                SYN_FrameInfo(0,"吃药时间到了，请吃药");
                Usart2_RxCounter = 0;
                memset(Usart2_RxBuff, 0, USART2_RXBUFF_SIZE);
            }
            
            if(strstr(Usart2_RxBuff,"sg90:off")){
                SG90=0;
                set_servo_angle(0);
                BEEP=0;
                BEEP_OUT(1);
                Usart2_RxCounter = 0;
                memset(Usart2_RxBuff, 0, USART2_RXBUFF_SIZE);
            }
            
            if(strstr(Usart2_RxBuff,"weight:on")){
                
                BEEP=1;
                BEEP_OUT(0);
                SYN_FrameInfo(0,"药品余量不足，请及时加药");
                Usart2_RxCounter = 0;
                memset(Usart2_RxBuff, 0, USART2_RXBUFF_SIZE);
            }
            
            if(strstr(Usart2_RxBuff,"weight:off")){
                
                BEEP=0;
                BEEP_OUT(1);
                Usart2_RxCounter = 0;
                memset(Usart2_RxBuff, 0, USART2_RXBUFF_SIZE);
            }
            
        }
				
				//管理USART2串口接收缓冲区 超过150字节则会情况 防止缓冲区溢出
				if(Usart2_RxCounter >= 150){
					Usart2_RxCounter = 0;
					memset(Usart2_RxBuff, 0, USART2_RXBUFF_SIZE);
				}
				
    vTaskDelay(500);
    }
}

/*
警告任务
功能：实现定时吃药提醒功能
逻辑：当SG90标志为1时，打开蜂鸣器并播放语音提醒；当SG90标志为0时，关闭蜂鸣器并控制舵机关闭出药口
注意：SG90标志由上位机控制，指示是否到达吃药时间
**/
void alert_task(void *argument)
{
    for(;;){
    
        if(SG90){  // SG90标志由上位机设置，指示是否到达吃药时间
            BEEP_OUT(0);  // 打开蜂鸣器报警
            BEEP = 1;  // 设置蜂鸣器状态标志
            
            // 播放语音提醒 
            SYN_FrameInfo(0,"约定的吃药时间已到，请按时服用药物");
            vTaskDelay(3000);  // 添加3秒延时，确保语音播报完成
        }else{
            BEEP=0;  // 关闭蜂鸣器标志
            BEEP_OUT(1);  // 关闭蜂鸣器
            
            set_servo_angle(0);  // 控制舵机关闭出药口 角度设为0
            BEEP_OUT(1);  // 确保蜂鸣器处于关闭状态
        
        }
            vTaskDelay(3000);  // 任务周期3秒
    }
    
}

void StartDefaultTask(void *argument)
{
    for(;;){
    

        vTaskDelay(3000);
    }
}

/**
按键控制任务
功能：处理按键事件，实现短按、长按、双击功能
逻辑：当检测到任何按键事件（短按、长按、双击）时，将SG90标志设为0，关闭舵机
用途：用于紧急关闭舵机，停止吃药提醒
*/
void Key_Task(void *pvParameters)
{
    uint8_t event, key_num;
    
    while(1)
    {
        for (key_num = 1; key_num <= 3; key_num++)
        {
            event = KEY_GetEvent(key_num, NULL);
            
            if (event != 0)
            {
                switch (event)
                {
                    case KEY_EVENT_SHORT_PRESS:
                        SG90 = 0;
                        break;
                        
                    case KEY_EVENT_LONG_PRESS:
                        SG90 = 0;
                        break;
                        
                    case KEY_EVENT_DOUBLE_CLICK:
                        SG90 = 0;
                        break;
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
 * @brief MX_FREERTOS_Init 函数
 * 
 * @功能: 系统级别初始化函数，负责初始化所有硬件模块并创建FreeRTOS任务
 * 
 * @初始化模块说明:
 *      1. OLED显示屏：初始化并显示初始化信息
 *      2. GPIO输出模块：控制蜂鸣器、LED等输出设备
 *      3. HX711称重传感器：测量药箱中药品重量
 *      4. SG90舵机：控制药箱出药口开关，初始角度设为0度
 *      5. DS1302实时时钟：提供实时时间和日期信息
 *      6. DHT11温湿度传感器：测量药箱内部温湿度
 *      7. WiFi模块：实现与云端的MQTT通信
 *      8. 按键模块：实现本地按键控制
 * 
 * @创建任务说明:
 *      1. Oled_Show：OLED显示任务，显示传感器数据和时间
 *      2. ESP_ReceiveTask：MQTT命令接收与处理任务
 *      3. ESP_SendTask：传感器数据采集与MQTT发布任务
 *      4. Sensor_Task：传感器数据采集任务
 *      5. StartDefaultTask：默认空闲任务
 *      6. Key_Task：按键控制任务，处理短按、长按、双击事件
 *      7. alert_task：警告任务，实现定时吃药提醒
 */
void MX_FREERTOS_Init(void)
{
    HAL_Delay(500);
    
    OLED_Init();
    OLED_ShowString(0, 16, "start init", OLED_8X16);
    OLED_Update();
    
    GPIO_OUT_Init();
    HX711_Init();
    sg90_Init();
    set_servo_angle(0);
    
    DS1302_GPIO_Init();
    
    if(dht11_init()==0)
    {
    }
    else
    {
        OLED_Clear();
        OLED_ShowString(0, 32, " Init Fail", 6);
        OLED_Update();
        HAL_Delay(500);
    }
    
    if(WiFi_init() == 0)
    {
        OLED_Clear();
        OLED_ShowString(0, 32, "wifi Init Fail", 6);
        OLED_Update();
    }
    
    KEY_Init();
    
    xTaskCreate(Oled_Show, "Oled_Show", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ESP_ReceiveTask, "ESP_ReceiveTask", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(ESP_SendTask, "ESP_SendTask", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(Sensor_Task, "Sensor_Task", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
    xTaskCreate(Key_Task, "Key_Task", 128, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(alert_task, "alert_task", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
    
    OLED_Clear();
}




