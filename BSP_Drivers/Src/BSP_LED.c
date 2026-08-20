#include "BSP_LED.h"

// 关于灯的函数 , BSP : Board Support Package:板级支持包
// flag: true -> 点亮，false->熄灭
void BSP_LED_Control(LED_t led, bool flag)
{
    GPIO_PinState state = flag ? GPIO_PIN_RESET : GPIO_PIN_SET;
    switch (led)
    {
    case LED1:
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_8, state);
        break;
    case LED2:
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_9, state);
        break;
    case LED3:
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, state);
        break;
    case LED4:
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_11, state);
        break;
    default:
        break;
    }
}

void BSP_LED_On(LED_t led)
{
    BSP_LED_Control(led, true);
}
void BSP_LED_Off(LED_t led)
{
    BSP_LED_Control(led, false);
}

void LED_Blink(LED_t led, uint8_t cnt, uint16_t delay)
{
    if (led == ALL)
    {
        LED_t leds[] = {LED1, LED2, LED3, LED4};
        int leds_num = sizeof(leds) / sizeof(leds[0]);
        do
        {
            // 先同时亮
            for (int i = 0; i < leds_num; i++)
            {
                BSP_LED_On(leds[i]);
            }
            HAL_Delay(delay);
            // 先同时灭
            for (int i = 0; i < leds_num; i++)
            {
                BSP_LED_Off(leds[i]);
            }
            HAL_Delay(delay);
        } while (cnt--);
    }
    else
    {
        for (int i = 0; i < cnt; i++)
        {
            BSP_LED_On(led);
            HAL_Delay(delay);

            BSP_LED_Off(led);
            HAL_Delay(delay);
        }
    }
}