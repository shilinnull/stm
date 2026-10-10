#include "APP_IC_test.h"

volatile static app_handler_t g_handler;
volatile static app_measure_t g_measure;

#define WARNING_LED_ON_DISTANCE_CM  10.0f
#define WARNING_LED_OFF_DISTANCE_CM 11.0f

static bool g_warning_led_on;

static void KeyPressCallback(uint8_t key, key_state_t state)
{
    // 如果按键被按下了
    if (state == KEY_STATE_PRESS)
    {
        switch (key)
        {
        case 0:
            HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_8);
            g_handler.configure.is_ultrasonic_ranging_enable = !g_handler.configure.is_ultrasonic_ranging_enable;
            break;
        case 1:
            HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
            g_handler.configure.is_usart_enable = !g_handler.configure.is_usart_enable;
            break;
        case 2:
            HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_10);
            g_handler.configure.is_warning_enable = !g_handler.configure.is_warning_enable;
            break;
        default:
            break;
        }
    }
    else if (state == KEY_STATE_RELEASE) // 按键被释放了
    {
        // DO Nothing
    }
    else
    {
    }
}

static void Trigger_10us_Pulse()
{
    g_measure.measure_done = false;                     // 每次开始测量，都认为没有测量完成
    __HAL_TIM_SET_COUNTER(g_handler.htim_ic, 0);        // 每一轮测距之前，都把计数器清零，这样就可以不用考虑溢出问题
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET); // 拉高PA1，开始触发
    HAL_TIM_Base_Start_IT(g_handler.htim_trigger);      // 启动TIM7，开始计时，够了10us，就会发送中断
}

static void Ultrasonic_CalcDistance(void)
{
    if (g_measure.measure_done)
    {
        uint32_t pulse_width = g_measure.falling_time - g_measure.rising_time;
        g_measure.distance_cm = (pulse_width * 0.034) / 2.0f; // 340 m/s = 0.034 cm/µs,就是340 * 100 / 1000 000
        if (g_measure.distance_cm >= 600.0f || g_measure.distance_cm < 2.0f)
        {
            g_measure.distance_cm = 0.0f; // 表示测距有问题
        }
    }
}

static void Print_Distance_Log()
{
    if (g_measure.distance_cm <= 2.0f) // 浮点数不要直接==比较
        return;
    printf(">>> 距离: %.02f cm\n", g_measure.distance_cm);
}

static void Warning_LED(void)
{
    if (g_measure.distance_cm >= 2.0f &&
        g_measure.distance_cm <= WARNING_LED_ON_DISTANCE_CM)
    {
        g_warning_led_on = true;
    }
    else if (g_measure.distance_cm >= WARNING_LED_OFF_DISTANCE_CM)
    {
        g_warning_led_on = false;
    }

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1,
                     g_warning_led_on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void App_Setup(TIM_HandleTypeDef *htim_trigger, TIM_HandleTypeDef *htim_key, TIM_HandleTypeDef *htim_ic, UART_HandleTypeDef *huart)
{
    if (!htim_key || !htim_ic || !huart)
        return;
    // 配置外部句柄
    g_handler.htim_trigger = htim_trigger;
    g_handler.htim_key = htim_key;
    g_handler.htim_ic = htim_ic;
    g_handler.huart = huart;
    BSP_Usart_Init(g_handler.huart);

    // 配置使能位
    g_handler.configure.is_ultrasonic_ranging_enable = false;
    g_handler.configure.is_usart_enable = false;
    g_handler.configure.is_warning_enable = false;

    // 配置测量参数
    g_measure.falling_time = g_measure.rising_time = 0;
    g_measure.distance_cm = 0.0f;
    g_measure.measure_done = true;
    g_warning_led_on = false;
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

    // 启动基本定时器，开始扫描key
    Key_Init(KeyPressCallback);                 // 设置按下回调
    HAL_TIM_Base_Start_IT(g_handler.htim_key); // 开始扫描

    // 开启输入捕获
    HAL_TIM_IC_Start_IT(g_handler.htim_ic, TIM_CHANNEL_3);
    HAL_TIM_IC_Start_IT(g_handler.htim_ic, TIM_CHANNEL_4);
    // 设置初始化完毕
    g_handler.is_init = true;
}

bool App_LoopOnce()
{
    // 1. 判定是否初始化
    if (!g_handler.is_init)
        return false;
    // 2. 扫描按键配置

    // 3. 根据配置信息进行工作
    // 先检测是否开启超声波探测
    if (g_handler.configure.is_ultrasonic_ranging_enable)
    {
        // 开始进行发送10us高电平
        Trigger_10us_Pulse();
        // 发送触发脉冲和收到echo回复脉冲是两码事，发送超声波之后，肯定在空气中要飞行一段时间，所以必须等
        // 而且要考虑超声波丢失的可能性,我们就采用极限情况4m来进行测算,这里直接HAL_Delay了
        HAL_Delay(30); // 30ms
        if (g_measure.measure_done) // 只有当他确实测量完成，才会走后续逻辑，否则什么也不做，返回之后继续测量
        {
            // 是否打印
            if (g_handler.configure.is_usart_enable)
                Print_Distance_Log();

            // 近距离告警闪灯
            if (g_handler.configure.is_warning_enable)
                Warning_LED();
            else
            {
                g_warning_led_on = false;
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
            }
        }
    }
    else if (g_warning_led_on)
    {
        g_warning_led_on = false;
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    }
    return true;
}

// TIM6定时器超时回调
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // 证明是TIM6超时了
    if (htim == g_handler.htim_key)
    {
        KEY_Scan(); // 扫描， 如果确实按下了就执行回调
    }
    // 证明是TIM7超时了
    if (htim == g_handler.htim_trigger)
    {
        // 拉低PA1，完成触发脉冲信号的发送
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); // 拉低PA1，完成触发
        // 关闭TIM7超时中断，下次按需在开启
        HAL_TIM_Base_Stop_IT(g_handler.htim_trigger);
    }
}

// TIM4 通道3/4输入捕获回调函数
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim == g_handler.htim_ic)
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) // 活跃通道
        {
            g_measure.rising_time = HAL_TIM_ReadCapturedValue(g_handler.htim_ic, TIM_CHANNEL_3);
        }
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
        {
            g_measure.falling_time = HAL_TIM_ReadCapturedValue(g_handler.htim_ic, TIM_CHANNEL_4);
            g_measure.measure_done = true; // 测量结束
            Ultrasonic_CalcDistance();    // 测算完毕就计算
        }
        else
        {
        }
    }
}
