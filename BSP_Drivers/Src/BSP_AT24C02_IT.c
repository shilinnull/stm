#include "BSP_AT24C02_IT.h"
#include <stdio.h>

eeprom_t at24c02;

static bool BSP_AT24C02_Ready()
{
    if (!at24c02.is_init)
        return false;
    return HAL_OK == HAL_I2C_IsDeviceReady(at24c02.hi2c, at24c02.dev_address, POLL_RETYIES, POLL_TIME_MS);

    // uint32_t start = HAL_GetTick();
    // HAL_I2C_IsDeviceReady(at24c02.hi2c, at24c02.dev_address, POLL_RETYIES, POLL_TIME_MS);
    // uint32_t end = HAL_GetTick();
    // printf("cast time: %d\n", end - start);
    // return true;
}

bool BSP_AT24C02_Init(I2C_HandleTypeDef *hi2c)
{
    if (!hi2c)
        return false;
    at24c02.hi2c = hi2c;
    at24c02.dev_address = DEV_ADDRESS << 1; // 左移1位
    at24c02.page_size = PAGE_SIZE;
    at24c02.is_init = false;

    // 初始化其他字段
    at24c02.status = EEPROM_IDLE;
    at24c02.buffer = NULL;
    at24c02.len = 0;
    at24c02.operator_type = NONE_OPERATOR;
    at24c02.user_callback = NULL;

    // 1. 产生起始
    // 2. 发送从设备地址 + w(0) = 8字节数据
    // 3. ACK -> 从设备活着呢！
    // 4. STOP
    if (HAL_OK == HAL_I2C_IsDeviceReady(at24c02.hi2c, at24c02.dev_address, SCAN_RETYIES, SCAN_TIME_MS))
    {
        at24c02.is_init = true;
        at24c02.status = EEPROM_READY;
    }

    return at24c02.is_init;
}

// 按字节，按page，按任意读
bool BSP_AT24C02_Read_Page_IT(uint16_t start_address, void *data_out, uint8_t data_len, user_callback_t cb)
{
    if (!at24c02.is_init || !data_out || data_len == 0)
    {
        if (cb)
            cb(false);
        return false;
    }

    if (start_address + data_len > DEV_TOTAL_SIZE)
    {
        if (cb)
            cb(false);
        return false;
    }

    if (at24c02.status == EEPROM_BUSY)
    {
        if (cb)
            cb(false);
        return false;
    }

    at24c02.status = EEPROM_BUSY;
    at24c02.buffer = data_out;
    at24c02.len = data_len;
    at24c02.operator_type = READ_OPERATOR;
    at24c02.user_callback = cb;

    // 这里有没有真正的读取数据呢？？没有！！使能中断，发起计划
    HAL_StatusTypeDef state = HAL_I2C_Mem_Read_IT(
        at24c02.hi2c, at24c02.dev_address,
        start_address,
        I2C_MEMADD_SIZE_8BIT,
        data_out,
        data_len);

    if (state != HAL_OK)
    {
        if (cb)
            cb(false);
        return false;
    }
    return true;
}

// 单页，页内写入
bool BSP_AT24C02_Write_Page_IT(uint16_t start_address, void *data_in, uint8_t data_len, user_callback_t cb)
{
    if (!at24c02.is_init || !data_in || data_len == 0)
    {
        if (cb)
            cb(false);
        return false;
    }

    // 检查写入是否跨页
    // 当前写入到那一页
    // uint16_t page_num = start_address / at24c02.page_size; // 不重要
    uint16_t page_offset = start_address % at24c02.page_size; // 确定的一页，写入位置的开始下标！

    if (page_offset + data_len > at24c02.page_size)
    {
        if (cb)
            cb(false);
        return false;
    }

    if (at24c02.status == EEPROM_BUSY)
    {
        if (cb)
            cb(false);
        return false;
    }

    at24c02.status = EEPROM_BUSY;
    at24c02.buffer = data_in;
    at24c02.len = data_len;
    at24c02.operator_type = WRTIE_OPERATOR;
    at24c02.user_callback = cb;

    // 这里有没有真正的写入数据呢？？没有！！使能中断，发起计划
    HAL_StatusTypeDef state = HAL_I2C_Mem_Write_IT(
        at24c02.hi2c,
        at24c02.dev_address,
        start_address,
        I2C_MEMADD_SIZE_8BIT,
        data_in,
        data_len);

    if (state != HAL_OK)
    {
        if (cb)
            cb(false);
        return false;
    }

    return true;

    // return BSP_AT24C02_Ready(); // 写入的时候，只是通信完成了，并不表示AT24C02真的写入到自己的存储区域了
}

static Status_t BSP_AT24C02_Status()
{
    return at24c02.status;
}

bool BSP_AT24C02_WaitWrite_Process(void)
{
    // 当前at24c02正在写入 && I2C write 通信成功
    if (at24c02.operator_type == WRTIE_OPERATOR && BSP_AT24C02_Status() == EEPROM_SUCCESS)
    {
        BSP_AT24C02_Ready();
        printf("设备写入等待完成...\n");
        at24c02.status = EEPROM_READY;
        return true;
    }
    return false;
}

// 读回调，走到这里，我们的数据已经被读完了！！
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (at24c02.hi2c->Instance == hi2c->Instance)
    {
        // 只处理读
        if (at24c02.operator_type == READ_OPERATOR)
        {
            at24c02.status = EEPROM_SUCCESS;
            if (at24c02.user_callback)
            {
                at24c02.user_callback(true);
                at24c02.user_callback = NULL;
                // at24c02.status = EEPROM_READY;
            }
        }
    }
}

// 写回调，走到这里，我们的数据已经被写完了吗？？完了！
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (at24c02.hi2c->Instance == hi2c->Instance)
    {
        // 只处理写
        if (at24c02.operator_type == WRTIE_OPERATOR)
        {
            at24c02.status = EEPROM_SUCCESS;
            if (at24c02.user_callback)
            {
                at24c02.user_callback(true);
                at24c02.user_callback = NULL;
            }
        }
    }
}

// 出错回调
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (at24c02.hi2c->Instance == hi2c->Instance)
    {
        at24c02.status = EEPROM_ERROR;
        if(at24c02.user_callback)
        {
            at24c02.user_callback(false);
            at24c02.user_callback = NULL;
        }
    }
}
