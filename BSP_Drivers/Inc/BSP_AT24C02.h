#ifndef __BSP_AT24C02_H
#define __BSP_AT24C02_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include "stm32f1xx_hal.h"

// 表示从设备
typedef struct
{
    I2C_HandleTypeDef *hi2c; // I2C2句柄指针
    uint16_t dev_address;    // 从设备地址
    uint8_t page_size;       // 页大小
    bool is_init;            // 是否被初始化
}eeprom_t;

#define DEV_ADDRESS 0x50
#define PAGE_SIZE 8
#define DEV_TOTAL_SIZE 256

#define SCAN_RETYIES 2 
#define SCAN_TIME_MS 5
#define POLL_RETYIES 10
#define POLL_TIME_MS 2
#define READ_TIMEOUT_MS 50
#define WRITE_TIMEOUT_MS 50

bool BSP_AT24C02_Init(I2C_HandleTypeDef *hi2c);
bool BSP_AT24C02_Read(uint16_t start_address, void *data_out, uint8_t data_len);
bool BSP_AT24C02_Write(uint16_t start_address, void *data_in, uint8_t data_len);

#ifdef __cplusplus
}
#endif

#endif