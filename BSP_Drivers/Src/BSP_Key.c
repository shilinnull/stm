#include "BSP_Key.h"

volatile key_t keys[KEY_NUM];
volatile KeyCallBack_t g_key_callback = NULL;

void Key_Init(KeyCallBack_t cb)
{
    for(int index = 0; index < KEY_NUM; index++)
    {
        keys[index].debouncecnt = 0;
        keys[index].state = KEY_STATE_RELEASE; // 默认就是松开的
    }
    // 设置回调
    g_key_callback = cb;
}

// 4个按键，编号依次是0,1,2,3
static uint8_t ReadKeyLevel(uint8_t key_id)
{
    switch(key_id)
    {
        case 0: return HAL_GPIO_ReadPin(KEY_PORT_0, KEY_PIN_0);
        case 1: return HAL_GPIO_ReadPin(KEY_PORT_1, KEY_PIN_1);
        case 2: return HAL_GPIO_ReadPin(KEY_PORT_2, KEY_PIN_2);
        case 3: return HAL_GPIO_ReadPin(KEY_PORT_3, KEY_PIN_3);
        default: return 1; // 默认就是外部拉高的，这里不清楚就默认了
    }
}

// 未来定时器中断到来，需要扫描所有的按键状态
// 注意: 定时间溢出周期是1ms
void KEY_Scan(void)
{
    // 遍历所有的按键
    for(int index = 0; index < KEY_NUM; index++)
    {
        // 临时读取当前引脚的电平-就是判断按键是否按下,注意，一次按下不算，还要考虑消抖问题
        uint8_t level = ReadKeyLevel(index);
        if(level == KEY_ACTIVE_LEVEL) // 说明按键此时按下了
        {
            // 消抖
            if(keys[index].debouncecnt < DEBOUNCE_MS)
            {
                // 什么都不做，就单存计数器++。因为TIM 中断是1ms触发一次，
                // 所以需要触发20次才算是一次有效按下，越过开始的20ms完成消抖
                keys[index].debouncecnt++;
            }
            if(keys[index].debouncecnt >= DEBOUNCE_MS && keys[index].state == KEY_STATE_RELEASE)
            {
                keys[index].state = KEY_STATE_PRESS; // 被按下
                // 既然被按下，就触发回调，注意，这里20次TIM溢出才会触发一次哦
                if(g_key_callback) g_key_callback(index, KEY_STATE_PRESS); // 指定按键被按下了
            }
        }
        else
        {
            // 当前按键可能被松开了,随着中断触发，递减计数器，完成松开消抖
            if(keys[index].debouncecnt > 0)
            {
                keys[index].debouncecnt--;
            }
            // 为什么增加该判断，按键有三种状态：1.按下 2. 松开 3.默认， 但是定时器未来可一直在溢出，未来情况3，就会走到这里，我们什么都不做即可
            if(keys[index].debouncecnt == 0 && keys[index].state == KEY_STATE_PRESS)
            {
                keys[index].state = KEY_STATE_RELEASE;
                if(g_key_callback) g_key_callback(index, KEY_STATE_RELEASE); // 指定按键被松开了
            }
        }
    }
}
