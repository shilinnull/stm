#ifndef __BSP_WS2812B_H
#define __BSP_WS2812B_H

#ifdef __cplusplus
extern "C"
{ // extern "C", 告诉编译器，头文件里的函数是用C语言规则编译的
#endif

#include "stm32f1xx_hal.h"
#include <stdbool.h>


// 0 码 和 1 码 PWM宽度
#define BIT0_PULSE 21
#define BIT1_PULSE 43
#define RESET_PULSE 0

#define ROWS 3
#define COLS 3

#define RESET_COUNT 100 // 80us-> 100 PWM 低电平周期

    // 站在用户视角，描述灯的颜色
    typedef struct
    {
        uint8_t r; // 0000 0000
        uint8_t g;
        uint8_t b;
    } rgb_t;

    // 站在用户视角，描述板子有几个灯
    typedef struct
    {
        rgb_t leds[ROWS][COLS];
    }rgb_led_group_t;

    // 站在定时器视角，描述一个灯对应的24个CCR的值，21or43->转化成为PWM波形对应的0,1码
    typedef struct 
    {
        uint16_t green_ccr_pulse[8];
        uint16_t red_ccr_pulse[8];
        uint16_t blue_ccr_pulse[8];
    }pulse_t;

    // 站在定时器视角, 要通过DMA搬迁的数据
    typedef struct
    {
        pulse_t leds[ROWS][COLS];
        uint16_t reset[RESET_COUNT];
    }pulse_group_t;



void BSP_ConvertRGB2Pulse(void);
// 设置一个灯的颜色
void BSP_WS2812BSetColor(uint8_t x, uint8_t y, rgb_t rgb);
// 重置所有的灯的颜色
void BSP_WS2812BResetColor(void);
void BSP_WS2812BInit(TIM_HandleTypeDef *htim);
bool BSP_WS2812BRefresh(void);
// For test
void BSP_WaitDMADone(void);
// For test
// DMA完成回到
void BSP_DMAPulseCplt(void);

#ifdef __cplusplus
}
#endif

#endif