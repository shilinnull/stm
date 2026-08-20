#include "BSP_AT24C02.h"

eeprom_t at24c02;

bool BSP_AT24C02_Init(I2C_HandleTypeDef *hi2c)
{
    if (!hi2c)
        return false;
    at24c02.hi2c = hi2c;
    at24c02.dev_address = DEV_ADDRESS << 1; // 左移1位
    at24c02.page_size = PAGE_SIZE;
    at24c02.is_init = false;

    // 1. 产生起始
    // 2. 发送从设备地址 + w(0) = 8字节数据
    // 3. ACK -> 从设备活着呢！
    // 4. STOP
    if (HAL_OK == HAL_I2C_IsDeviceReady(at24c02.hi2c, at24c02.dev_address, SCAN_RETYIES, SCAN_TIME_MS))
    {
        at24c02.is_init = true;
    }

    return at24c02.is_init;
}

// 按字节，按page，按任意读
bool BSP_AT24C02_Read(uint16_t start_address, void *data_out, uint8_t data_len)
{
    if (!at24c02.is_init || !data_out || data_len == 0)
        return false;

    if (start_address + data_len > DEV_TOTAL_SIZE)
    {
        return false;
    }

    HAL_StatusTypeDef state = HAL_I2C_Mem_Read(
        at24c02.hi2c, at24c02.dev_address,
        start_address,
        I2C_MEMADD_SIZE_8BIT,
        data_out,
        data_len,
        READ_TIMEOUT_MS);
    return state == HAL_OK;
}

static bool BSP_AT24C02_Ready()
{
    if(!at24c02.is_init)
        return false;
    return HAL_OK == HAL_I2C_IsDeviceReady(at24c02.hi2c, at24c02.dev_address, POLL_RETYIES, POLL_TIME_MS);
}

// 单页，页内写入
static bool BSP_AT24C02_Write_Page(uint16_t start_address, void *data_in, uint8_t data_len)
{
    if(!at24c02.is_init || !data_in || data_len == 0)
        return false;
    
    // 检查写入是否跨页
    // 当前写入到那一页
    // uint16_t page_num = start_address / at24c02.page_size; // 不重要
    uint16_t page_offset = start_address % at24c02.page_size; // 确定的一页，写入位置的开始下标！

    if(page_offset + data_len > at24c02.page_size)
    {
        return false;
    }

    HAL_StatusTypeDef state = HAL_I2C_Mem_Write(
        at24c02.hi2c,
        at24c02.dev_address,
        start_address,
        I2C_MEMADD_SIZE_8BIT,
        data_in,
        data_len,
        WRITE_TIMEOUT_MS);
    if(state != HAL_OK)
        return false; 
    
    return BSP_AT24C02_Ready();
}

// 支持任意长度的写入，自动跨页
bool BSP_AT24C02_Write(uint16_t start_address, void *data_in, uint8_t data_len)
{
    if(!at24c02.is_init || !data_in || data_len == 0)
        return false;
    if(start_address + data_len > DEV_TOTAL_SIZE)
        return false;
    
    uint16_t bytes_written = 0; // 已经写入了多少个字节
    while(bytes_written < data_len)
    {
        // 数据还有多少没有发送
        uint16_t data_left = data_len - bytes_written;

        // 必须按照页来写，当前页剩余多少空间
        uint16_t new_start_address = start_address + bytes_written;
        // uint16_t current_page_num = new_start_address / at24c02.page_size;
        uint16_t page_offset = new_start_address % at24c02.page_size;
        uint16_t page_space = at24c02.page_size - page_offset;

        // 数据还有多少没有发送 -> 当前页剩余多少空间
        // 一定能写入当前页的剩余空间吗？不一定！！
        // 计算本次写入量
        // 1. 数据还有多少没有发送 <= 当前页剩余多少空间 = 数据还有多少
        // 2. 数据还有多少没有发送 > 当前页剩余多少空间 = 剩余多少空间
        uint16_t write_bytes = data_left;
        if(write_bytes > page_space)
        {
            write_bytes = page_space;
        }

        if(!BSP_AT24C02_Write_Page(new_start_address, (uint8_t*)data_in+bytes_written, write_bytes))
        {
            return false;
        }

        bytes_written += write_bytes;
    }

    return true;
}