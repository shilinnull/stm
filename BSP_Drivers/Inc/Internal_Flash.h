#ifndef __INTERNAL_FLASH_H
#define __INTERNAL_FLASH_H


#ifdef __cplusplus
extern "C" { // extern "C", 告诉编译器，头文件里的函数是用C语言规则编译的
#endif

#include <stdio.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"


void InternalFlash_AutoTest(void);
void InternalFlash_Read(uint32_t start_address, uint8_t *outbuffer, uint32_t len);
bool InternalFlash_ErasePages(uint32_t start_page_address, uint32_t nb_pages);
bool InternalFlash_Program(uint32_t start_address, const uint8_t *inbuffer, uint32_t len);
bool InternalFlash_Write(uint32_t start_address, const uint8_t *inbuffer, uint32_t len); // 写前会擦除
bool InternalFlash_MassErase(void);

#ifdef __cplusplus
}
#endif

#endif