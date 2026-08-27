#ifndef __BSP_AT24C02_H
#define __BSP_AT24C02_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include "stm32f1xx_hal.h"

// 设备状态
typedef enum
{
    EEPROM_IDLE = 0, // 空闲，没有被初始化
    EEPROM_READY,    // 就绪状态
    EEPROM_BUSY,     // 忙
    EEPROM_SUCCESS,  // 成功
    EEPROM_ERROR     // 错误
}Status_t;

#define NONE_OPERATOR 0
#define READ_OPERATOR 1
#define WRTIE_OPERATOR 2

typedef void(*user_callback_t)(bool); // 函数指针类型

// 表示从设备
typedef struct
{
    I2C_HandleTypeDef *hi2c; // I2C2句柄指针
    uint16_t dev_address;    // 从设备地址
    uint8_t page_size;       // 页大小
    bool is_init;            // 是否被初始化

    // 添加中断标志位，其实就是全局变量
    volatile Status_t status;
    uint8_t *buffer;
    uint8_t len;
    volatile uint8_t operator_type;  // 操作类型
    user_callback_t user_callback;
}eeprom_t;

#define DEV_ADDRESS 0x50
#define PAGE_SIZE 8
#define DEV_TOTAL_SIZE 256

#define SCAN_RETYIES 2 
#define SCAN_TIME_MS 5
#define POLL_RETYIES 10
#define POLL_TIME_MS 2

bool BSP_AT24C02_Init(I2C_HandleTypeDef *hi2c);
bool BSP_AT24C02_Read_Page_DMA(uint16_t start_address, void *data_out, uint8_t data_len, user_callback_t cb);
bool BSP_AT24C02_Write_Page_DMA(uint16_t start_address, void *data_in, uint8_t data_len, user_callback_t cb);
bool BSP_AT24C02_WaitWrite_Process(void);

#ifdef __cplusplus
}
#endif

#endif