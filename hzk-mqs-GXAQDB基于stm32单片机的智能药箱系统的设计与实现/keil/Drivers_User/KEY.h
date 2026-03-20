#ifndef __KEY_H
#define __KEY_H
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "timers.h"
#include "oled.h"
#define KEY1_PIN      GPIO_PIN_13
#define KEY1_PORT     GPIOB

#define KEY2_PIN      GPIO_PIN_13
#define KEY2_PORT     GPIOB

#define KEY3_PIN      GPIO_PIN_13
#define KEY3_PORT     GPIOB

// 消抖时间(ms)
#define KEY_DEBOUNCE_TIME 20
// 长按时间(ms)
#define KEY_LONG_PRESS_TIME 1000
// 双击间隔时间(ms)
#define KEY_DOUBLE_CLICK_TIME 500


// 按键事件类型
#define KEY_EVENT_SHORT_PRESS 1
#define KEY_EVENT_LONG_PRESS  2
#define KEY_EVENT_DOUBLE_CLICK 3

// 按键事件结构体
typedef struct {
    uint8_t key_num;  // 按键编号(1,2,3)
    uint8_t event;    // 事件类型(1:短按, 2:长按, 3:双击)
} KeyEvent_t;

// 按键扫描定时器
extern TimerHandle_t xKeyScanTimer;

void KEY_Init(void);
uint8_t KEY_GetEvent(uint8_t key_num, uint8_t *param);
void KEY_ScanTask(void *argument);
void vKeyScanTimerCallback(TimerHandle_t xTimer);

#endif

//#ifndef __KEY_H
//#define __KEY_H
//#include "stm32f1xx_hal.h"
//#include "FreeRTOS.h"
//#include "queue.h"

//#define KEY1_PIN      GPIO_PIN_7
//#define KEY1_PORT     GPIOB

//#define KEY2_PIN      GPIO_PIN_6
//#define KEY2_PORT     GPIOB

//#define KEY3_PIN      GPIO_PIN_4
//#define KEY3_PORT     GPIOB


//void KEY_Init(void);
//unsigned char KeyGetNum(void);

//#endif

