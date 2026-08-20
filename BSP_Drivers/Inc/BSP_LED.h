#ifndef __BSP_LED_H
#define __BSP_LED_H


#ifdef __cplusplus
extern "C" { 
#endif
#include <stdbool.h>
#include "stm32f1xx_hal.h"


typedef enum
{
 LED1 = 1,
 LED2,
 LED3,
 LED4,
 ALL
} LED_t;

void BSP_LED_On(LED_t led);
void BSP_LED_Off(LED_t led);
void LED_Blink(LED_t led, uint8_t cnt, uint16_t delay);

#ifdef __cplusplus
}
#endif

#endif