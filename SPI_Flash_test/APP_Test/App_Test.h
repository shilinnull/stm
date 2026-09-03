#ifndef ___APP_TEST_H
#define ___APP_TEST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "BSP_W25Q64.h"

    void Test_W25Q64_Driver_Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *port, uint16_t pin);
    void Test_W25Q64_Driver_Reg1(void);
    void Test_W25Q64_Driver_Write_Enable_Disable(void);
    void Test_W25Q64_Driver_Erase_Read(void);
    void Test_W25Q64_Driver_ChipErase_Any_Read_Write(void);
    void For_SPI_Test(SPI_HandleTypeDef *hspi, GPIO_TypeDef *port, uint16_t pin);
#ifdef __cplusplus
}
#endif

#endif