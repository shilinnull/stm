#ifndef __BSP_W25Q64_H
#define __BSP_W25Q64_H


#ifdef __cplusplus
    extern "C" {
#endif


#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CMD_W25QXX_JEDEC_ID 0x9F
#define CMD_W25QXX_READ_STATUS_REG1 0x05
#define CMD_W25QXX_WRITE_ENABLE 0x06
#define CMD_W25QXX_WRITE_DISABLE 0x04
#define CMD_W25QXX_SECTOR_ERASE 0x20
#define CMD_W25QXX_READ_DATA 0x03
#define CMD_W25QXX_PAGE_PROGRAM 0x02
#define CMD_W25QXX_CHIP_ERASE 0xC7

// 设置BUSY检测位
#define W25Qxx_STATUS_BUSY 0x01 // 0000 0001
#define W25Qxx_STATUS_WEL  0x02 // 0000 0010
#define SECTOR_ADDRESS_MASK 0xFFF

typedef struct 
{
    uint32_t page_size;      // 页大小
    uint32_t sector_size;    // 扇区大小
    uint32_t block_size;     // 块大小
    uint32_t mem_total_size; // 总大小
    uint32_t chip_id;        // 芯片ID
    SPI_HandleTypeDef *hspi; // spi句柄指针
    GPIO_TypeDef *gpio_port; // 从设备选择引脚端口GPIOE
    uint16_t nss_pin;        // 片选针脚
}spi_flash_t;


bool BSP_W25Qxx_Init(spi_flash_t *init);
uint32_t BSP_W25Qxx_ID(void);
uint8_t BSP_W25Qxx_Statue1_Reg(void);
void BSP_W25Qxx_Write_Enable(void);
void BSP_W25Qxx_Write_Disable(void);
void BSP_W25Qxx_Waitfor_Write_Complete(void);
void BSP_W25Qxx_Waitfor_Write_Complete_V2(void);
void BSP_W25Qxx_Sector_Erase(uint32_t sector_address);
void BSP_W25Qxx_Chip_Erase(void);
// void BSP_W25Qxx_Block_Erase(void);
bool BSP_W25Qxx_Page_Program(uint32_t page_address, uint16_t page_inner_offset, uint8_t *wbuffer, uint16_t wsize);
bool BSP_W25Qxx_Read_Data(uint32_t read_address, uint8_t *rbuffer, uint32_t rsize);
bool BSP_W25Qxx_Write_Data(uint32_t write_address, uint8_t *wbuffer, uint32_t wsize);

#ifdef __cplusplus
}
#endif

#endif