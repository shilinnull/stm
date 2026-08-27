#include "BSP_WS2812B.h"

// 标志位，DMA
static volatile bool g_is_DMA_done = false;

// 指向全局的TIM3句柄
static TIM_HandleTypeDef *ghtim = NULL;

// 对应板子的灯，内部设置
static rgb_led_group_t rgb_led_group = {0};

// 全局变量, DMA要搬迁的数据
static pulse_group_t pulse_group = {0};



static void BSP_ConvertRGB2PulseHelper(pulse_group_t *pulse_group_dst, rgb_led_group_t *rgb_group_src)
{
    if (!pulse_group_dst || !rgb_group_src)
    {
        return;
    }

    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            // 转化一个灯
            // 一个灯 原始数据
            rgb_t rgb = rgb_group_src->leds[i][j];
            // 一个灯 目标数据
            pulse_t *ppulse = &(pulse_group_dst->leds[i][j]);
            // 扫描比特位，设置ccr的值
            for (int pos = 0; pos < 8; pos++)
            {
                // G
                if (rgb.g & (1 << (8 - pos - 1)))
                {
                    ppulse->green_ccr_pulse[pos] = BIT1_PULSE;
                }
                else
                {
                    ppulse->green_ccr_pulse[pos] = BIT0_PULSE;
                }

                // R
                if (rgb.r & (1 << (8 - pos - 1)))
                {
                    ppulse->red_ccr_pulse[pos] = BIT1_PULSE;
                }
                else
                {
                    ppulse->red_ccr_pulse[pos] = BIT0_PULSE;
                }

                // B
                if (rgb.b & (1 << (8 - pos - 1)))
                {
                    ppulse->blue_ccr_pulse[pos] = BIT1_PULSE;
                }
                else
                {
                    ppulse->blue_ccr_pulse[pos] = BIT0_PULSE;
                }
            }
        }
    }

    // 处理最后复位的部分reset
    for (int i = 0; i < RESET_COUNT; i++)
    {
        pulse_group_dst->reset[i] = RESET_PULSE;
    }
}

void BSP_ConvertRGB2Pulse(void)
{
    return BSP_ConvertRGB2PulseHelper(&pulse_group, &rgb_led_group);
}

// 设置一个灯的颜色
void BSP_WS2812BSetColor(uint8_t x, uint8_t y, rgb_t rgb)
{
    if (x >= ROWS || y >= COLS)
        return;
    rgb_led_group.leds[x][y].r = rgb.r;
    rgb_led_group.leds[x][y].g = rgb.g;
    rgb_led_group.leds[x][y].b = rgb.b;
}

// 重置所有的灯的颜色
void BSP_WS2812BResetColor(void)
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            rgb_led_group.leds[i][j].r = 0;
            rgb_led_group.leds[i][j].g = 0;
            rgb_led_group.leds[i][j].b = 0;
        }
    }
}

void BSP_WS2812BInit(TIM_HandleTypeDef *htim)
{
    if(!htim)
        return;
    ghtim = htim;
}

bool BSP_WS2812BRefresh(void)
{
    // 检测是否初始化
    if(ghtim == NULL)
        return false;

    HAL_StatusTypeDef state = HAL_TIM_PWM_Start_DMA(ghtim, TIM_CHANNEL_3,\
            (uint32_t*)&pulse_group,\
            sizeof(pulse_group)/sizeof(uint16_t));
    if(state != HAL_OK)
    {
        return false;
    }
    else
    {
        g_is_DMA_done = false;
    }

    return true;
}

// For test
void BSP_WaitDMADone(void)
{
    while(!g_is_DMA_done);
}

// For test
// DMA完成回到
void BSP_DMAPulseCplt(void)
{
    g_is_DMA_done = true;
}