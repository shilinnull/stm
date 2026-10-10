#ifndef __BSP_IC_TEST_APP_H
#define __BSP_IC_TEST_APP_H

#ifdef __cplusplus
extern "C"
{ // extern "C", 告诉编译器，头文件里的函数是用C语言规则编译的
#endif

#include "BSP_LED.h"
#include "BSP_Usart_Redir.h"
#include "BSP_Key.h"

#include <stdbool.h>

    typedef struct app_configure
    {
        bool is_ultrasonic_ranging_enable; // 超声波测距是否开启
        bool is_usart_enable;              // 串口打印是否开启
        bool is_warning_enable;            // 点灯告警是否开启
    } app_configure_t;

    typedef struct app_handler
    {
        TIM_HandleTypeDef *htim_trigger; // 基本定时器TIM7句柄指针，未来触发HC-SR04发送超声波
        TIM_HandleTypeDef *htim_key; // 基本定时器TIM6句柄指针TIM6，未来进行按键扫描功能
        TIM_HandleTypeDef *htim_ic; // 输入捕获句柄指针，双通道直接间接模式，未来捕获HC-SR04 echo 返回的脉冲
        UART_HandleTypeDef *huart;
        app_configure_t configure;
        bool is_init;
    } app_handler_t;

    // 测量结构体
    typedef struct app_measure
    {
        uint32_t rising_time;  // 捕获上升沿时刻
        uint32_t falling_time; // 捕获下降沿时刻
        bool measure_done;    // 测量是否结束
        float distance_cm;    // 距离，单位cm
    } app_measure_t;

    void App_Setup(TIM_HandleTypeDef *htim_trigger, TIM_HandleTypeDef *htim_key, TIM_HandleTypeDef *htim_ic, UART_HandleTypeDef *huart);
    bool App_LoopOnce();

#ifdef __cplusplus
}
#endif

#endif
