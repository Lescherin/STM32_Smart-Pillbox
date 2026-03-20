#include "key.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "oled.h"

/**
KEY按钮
功能用途 ：
功能用途 ：
- 提供本地操作接口，可进行手动控制
- 支持短按、长按、双击等多种操作模式
- 用于紧急关闭药箱、确认服药、手动出药等功能
引脚连接 ：
- KEY1引脚： PAx
- KEY2引脚： PAy
- KEY3引脚： PAz
通信方式 ：
- GPIO输入模式，上拉电阻
- 采用定时器中断扫描方式实现按键检测
- 支持消抖处理和多种按键事件识别
*/
// 按键扫描定时器
TimerHandle_t xKeyScanTimer;

// 按键状态枚举
typedef enum {
    KEY_STATE_IDLE,                 // 空闲状态
    KEY_STATE_PRESSED,              // 按下状态
    KEY_STATE_RELEASED_SHORT,       // 短按释放状态
    KEY_STATE_WAIT_DOUBLE,          // 等待双击状态
    KEY_STATE_DOUBLE_PRESSED,       // 双击按下状态
    KEY_STATE_WAIT_LONG_RELEASE,    // 等待长按释放状态
    KEY_STATE_RELEASED_LONG         // 长按释放后状态
} KeyState_t;

static KeyState_t key_states[3] = {KEY_STATE_IDLE, KEY_STATE_IDLE, KEY_STATE_IDLE};
static TickType_t key_press_time[3] = {0};
static TickType_t key_release_time[3] = {0};
static uint8_t key_events[3] = {0};    // 存储待处理的事件
static uint8_t key_event_params[3] = {0}; // 存储事件参数

// 按键初始化
void KEY_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin  = KEY1_PIN;
    HAL_GPIO_Init(KEY1_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin  = KEY2_PIN;
    HAL_GPIO_Init(KEY2_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Pin  = KEY3_PIN;
    HAL_GPIO_Init(KEY3_PORT, &GPIO_InitStruct);

    // 创建按键扫描定时器
    xKeyScanTimer = xTimerCreate("KeyScanTimer", pdMS_TO_TICKS(KEY_DEBOUNCE_TIME), pdTRUE, NULL, vKeyScanTimerCallback);
    if (xKeyScanTimer != NULL) {
        // 启动定时器
        xTimerStart(xKeyScanTimer, 0);
    }
}

// 获取按键事件
uint8_t KEY_GetEvent(uint8_t key_num, uint8_t *param)
{
    if (key_num < 1 || key_num > 3) return 0;
    
    uint8_t event = key_events[key_num-1];
    if (event != 0) {
        // 如果有事件，返回事件类型并清除事件标志
        if (param != NULL) {
            *param = key_event_params[key_num-1];
        }
        key_events[key_num-1] = 0;
    }
    return event;
}

void KEY_ScanTask(void *argument)
{
    // 扫描任务暂时不使用
}

// 按键扫描定时器回调
void vKeyScanTimerCallback(TimerHandle_t xTimer)
{
    uint8_t current_states[3];
    TickType_t current_time = xTaskGetTickCount();

    // 获取当前按键状态
    current_states[0] = HAL_GPIO_ReadPin(KEY1_PORT, KEY1_PIN);
    current_states[1] = HAL_GPIO_ReadPin(KEY2_PORT, KEY2_PIN);
    current_states[2] = HAL_GPIO_ReadPin(KEY3_PORT, KEY3_PIN);

    for (int i = 0; i < 3; i++) {
        switch (key_states[i]) {
            case KEY_STATE_IDLE:
                if (current_states[i] == 0) {
                    // 按键按下
                    key_press_time[i] = current_time;
                    key_states[i] = KEY_STATE_PRESSED;
                    // 清除之前的事件标志
                    key_events[i] = 0;
                }
                break;
                
            case KEY_STATE_PRESSED:
                if (current_states[i] == 1) {
                    // 按键释放
                    key_release_time[i] = current_time;
                    TickType_t press_duration = key_release_time[i] - key_press_time[i];
                    
                    if (press_duration < pdMS_TO_TICKS(KEY_LONG_PRESS_TIME)) {
                        // 短按或可能的双击
                        key_states[i] = KEY_STATE_RELEASED_SHORT;
                        // 记录短按事件
                        key_events[i] = KEY_EVENT_SHORT_PRESS;
                        key_event_params[i] = i + 1;
                    } else {
                        // 长按释放，进入等待长按释放状态
                        key_states[i] = KEY_STATE_WAIT_LONG_RELEASE;
                    }
                } else {
                    // 持续按下，检查是否达到长按时间
                    TickType_t press_duration = current_time - key_press_time[i];
                    if (press_duration >= pdMS_TO_TICKS(KEY_LONG_PRESS_TIME)) {
                        // 长按事件
                        key_events[i] = KEY_EVENT_LONG_PRESS;
                        key_event_params[i] = i + 1;
                        key_states[i] = KEY_STATE_WAIT_LONG_RELEASE;
                    }
                }
                break;
                
            case KEY_STATE_RELEASED_SHORT:
                if (current_time - key_release_time[i] > pdMS_TO_TICKS(KEY_DOUBLE_CLICK_TIME)) {
                    // 双击超时，确认是短按事件，保持事件标志不变
                    key_states[i] = KEY_STATE_IDLE;
                } else if (current_states[i] == 0) {
                    // 检测到第二次按下，触发双击
                    key_press_time[i] = current_time;
                    key_states[i] = KEY_STATE_DOUBLE_PRESSED;
                    // 覆盖短按事件为双击事件
                    key_events[i] = KEY_EVENT_DOUBLE_CLICK;
                    key_event_params[i] = i + 1;
                }
                break;
                
            case KEY_STATE_WAIT_DOUBLE:
                if (current_states[i] == 0) {
                    // 检测到第二次按下，触发双击
                    key_press_time[i] = current_time;
                    key_states[i] = KEY_STATE_DOUBLE_PRESSED;
                    // 覆盖短按事件为双击事件
                    key_events[i] = KEY_EVENT_DOUBLE_CLICK;
                    key_event_params[i] = i + 1;
                } else if (current_time - key_release_time[i] > pdMS_TO_TICKS(KEY_DOUBLE_CLICK_TIME)) {
                    // 双击超时，确认是短按
                    key_states[i] = KEY_STATE_IDLE;
                }
                break;
                
            case KEY_STATE_DOUBLE_PRESSED:
                if (current_states[i] == 1) {
                    // 双击释放，回到空闲状态
                    key_states[i] = KEY_STATE_IDLE;
                }
                break;
                
            case KEY_STATE_WAIT_LONG_RELEASE:
                if (current_states[i] == 1) {
                    // 长按释放确认，进入长按释放后状态
                    key_states[i] = KEY_STATE_RELEASED_LONG;
                } else {
                    // 按键仍被按下，保持当前状态
                }
                break;
                
            case KEY_STATE_RELEASED_LONG:
                if (current_time - key_release_time[i] > pdMS_TO_TICKS(200)) {
                    // 长按释放后延时，确保稳定，回到空闲状态
                    key_states[i] = KEY_STATE_IDLE;
                    // 清除事件标志
                    key_events[i] = 0;
                } else if (current_states[i] == 0) {
                    // 重新按下
                    key_press_time[i] = current_time;
                    key_states[i] = KEY_STATE_PRESSED;
                    // 清除之前的事件标志
                    key_events[i] = 0;
                }
                break;
        }
    }
}

