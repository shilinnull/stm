#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#ifdef __cplusplus
extern "C" { // extern "C", 告诉编译器，头文件里的函数是用C语言规则编译的
#endif

#include "stm32f1xx_hal.h"

typedef enum
{
    KEY_STATE_PRESS = 0,   // 按键按下
    KEY_STATE_RELEASE = 1  // 按键松开
}key_state_t;

typedef struct key
{
    key_state_t state; // 针脚电平
    uint16_t debouncecnt; // 消抖计数器
}key_t;

// 未来按键按下或者松开要执行的回调
typedef void (*KeyCallBack_t)(uint8_t key, key_state_t state);

// 我们板子上按键和GPIO的关系
#define KEY_NUM 4      // 4个按键
#define DEBOUNCE_MS 20 // 消抖时间或者次数

#define KEY_PORT_0 GPIOG
#define KEY_PIN_0  GPIO_PIN_10

#define KEY_PORT_1 GPIOG
#define KEY_PIN_1  GPIO_PIN_6

#define KEY_PORT_2 GPIOG
#define KEY_PIN_2  GPIO_PIN_8

#define KEY_PORT_3 GPIOG
#define KEY_PIN_3  GPIO_PIN_7

#define KEY_ACTIVE_LEVEL 0 // 按键活跃就是0，即按下接地

void Key_Init(KeyCallBack_t cb);
void KEY_Scan(void);

#ifdef __cplusplus
}
#endif

#endif
