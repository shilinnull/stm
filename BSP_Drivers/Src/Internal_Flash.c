#include "Internal_Flash.h"

// 2KB对齐， 2进制低11位是0
const uint32_t g_page_size = FLASH_PAGE_SIZE;
const uint32_t g_page_offset_mask = (FLASH_PAGE_SIZE - 1);
const uint32_t g_page_base_mask = ~g_page_offset_mask;

// 读取flash
void InternalFlash_Read(uint32_t start_address, uint8_t *outbuffer, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        outbuffer[i] = *(volatile uint8_t *)(start_address + i);
    }
}

// 页擦除
// 擦除的页的起始地址，必须是2KB对齐
bool InternalFlash_ErasePages(uint32_t start_page_address, uint32_t nb_pages)
{
    if (0 == nb_pages)
        return false;
    // 没有对齐
    if (start_page_address & g_page_offset_mask)
        return false;

    bool result = false;
    // 起始地址2KB对齐
    FLASH_EraseInitTypeDef erase_init = {0};
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.PageAddress = start_page_address;
    erase_init.NbPages = nb_pages;

    uint32_t pages_error = 0;
    HAL_FLASH_Unlock();
    result = (HAL_FLASHEx_Erase(&erase_init, &pages_error) == HAL_OK);
    HAL_FLASH_Lock();

    return result;
}

static inline uint16_t PackHalfWord(const uint8_t *start_address, uint32_t remain)
{
    // 转成小端, 永远只有0 1 下标
    if (remain == 0)
        return 0;
    if (remain == 1)
        return (uint16_t)start_address[0];               // 0000 0000 XXXX XXXX
    return (start_address[0] | (start_address[1] << 8)); // 1111 1111 0000 0000
}

// 编程，不考虑擦除
bool InternalFlash_Program(uint32_t start_address, const uint8_t *inbuffer, uint32_t len)
{
    if (NULL == inbuffer || 0 == len)
        return false;
    if (start_address & 0x1)
        return false;
    uint32_t i = 0;
    HAL_FLASH_Unlock();
    for (; i < len; i += 2)
    {
        // 1. 两个字节拼接uint16_t, 如果有的话
        uint16_t data = PackHalfWord(inbuffer + i, len - i);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, start_address + i, data) != HAL_OK)
        {
            break;
        }
    }
    HAL_FLASH_Lock();

    if (i < len)
        return false;
    return true;
}

// 写入地址向下2KB对齐
static inline uint32_t AlignDown2Page(uint32_t address)
{
    return (address & g_page_base_mask);
}

static inline uint32_t AlignUp2Page(uint32_t address)
{
    return ((address + g_page_size - 1) & g_page_base_mask);
}

// 写入，先擦除
bool InternalFlash_Write(uint32_t start_address, const uint8_t *inbuffer, uint32_t len)
{
    if (NULL == inbuffer || len == 0)
        return false;
    if (start_address & 0x1)
        return false;

    // 1. 计算擦除page的个数
    // 1.1 起始页地址
    uint32_t start_page = AlignDown2Page(start_address);
    // 1.2 结束页地址
    uint32_t end_page = AlignUp2Page(start_address + len);
    // 1.3 涉及到要擦除的页个数
    uint32_t nb_pages = (end_page - start_page) / g_page_size;

    // 2. 擦除 && 编程
    bool result = (InternalFlash_ErasePages(start_page, nb_pages) && InternalFlash_Program(start_address, inbuffer, len));
    return result;
}

// 整片擦除
bool InternalFlash_MassErase(void)
{
    FLASH_EraseInitTypeDef erase_init = {0};
    erase_init.TypeErase = FLASH_TYPEERASE_MASSERASE; // 整片
    erase_init.NbPages = 0;                           // 忽略
    erase_init.PageAddress = 0;                       // 忽略
    erase_init.Banks = FLASH_BANK_1;

    uint32_t page_error = 0;

    // 开始擦除
    HAL_FLASH_Unlock();
    HAL_StatusTypeDef state = HAL_FLASHEx_Erase(&erase_init, &page_error);
    HAL_FLASH_Lock();

    return state == HAL_OK;
}

///////////////////////////// For Test ///////////////////////////////////////////

static void TestPrintPage(uint32_t address)
{
    uint8_t buffer[16]; // 不要在函数中定义大大数组！！
    for (uint32_t i = 0; i < 128; i++)
    {
        printf(">>> %03d: ", i);
        InternalFlash_Read(address + i * 16, buffer, 16);
        for (uint32_t j = 0; j < 16; j++)
        {
            printf("0x%02x ", buffer[j]);
        }
        printf("\n");
    }
}

static void TestRead(void)
{
    printf(">>> 测试读取, Flash(0x0800 0000) 16 bytes <<<\n");
    uint8_t buffer[16] = {0};
    uint32_t len = sizeof(buffer) / sizeof(buffer[0]);
    uint32_t internalflash_start_address = 0x08000000;

    InternalFlash_Read(internalflash_start_address, buffer, len);

    for (uint32_t i = 0; i < len; i++)
    {
        printf("0x%02x ", buffer[i]);
    }
    printf("\n");
}

static void TestPageErase(void)
{
    printf(">>> 测试页擦除, Flash(0x0807F000) 2KB <<<\n");
    // 擦除，擦除倒数第二页
    uint32_t target_page = 0x0807F000;
    if (InternalFlash_ErasePages(target_page, 1))
    {
        printf("TestPageErase ... Pass!\n");
    }
    else
    {
        printf("TestPageErase ... Failed!\n");
    }

    // 获取倒数第二页并打印
    // TestPrintPage(target_page);
}

static void TestProgram(void)
{
    printf(">>> 测试页编程, Flash(0x0807F000) 2KB <<<\n");

    uint8_t buffer[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    uint32_t target_page = 0x0807F000;
    if (InternalFlash_Program(target_page, buffer, 16))
    {
        printf("TestProgram ... Pass!\n");
    }
    else
    {
        printf("TestProgram ... Failed!\n");
    }
    // 获取倒数第二页并打印
    // TestPrintPage(target_page);
}

static void TestWrite(void)
{
    printf(">>> 测试写入(擦除版), Flash(0x0807F000) 2KB <<<\n");

    uint32_t target_page = 0x0807E800; // 倒数第三个页
    uint8_t buffer[] = {0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
    if (InternalFlash_Write(target_page + 16, buffer, 16))
    {
        printf("TestWrite ... Pass!\n");
    }
    else
    {
        printf("TestWrite ... Failed!\n");
    }
    // 获取倒数第3页并打印
    // TestPrintPage(target_page);
}

static void TestMassErase(void)
{
    printf(">>> 测试整片擦除 <<<\n");
    printf(">>> 整片擦除开始!\n");
    InternalFlash_MassErase();
    printf(">>> 整片擦除结束!\n");
}

#define _DEBUG_

void InternalFlash_AutoTest(void)
{
#ifdef _DEBUG_
    TestRead();
    HAL_Delay(1000);

    TestPageErase();
    HAL_Delay(1000);

    TestProgram();
    HAL_Delay(1000);

    TestWrite();
    HAL_Delay(1000);

    TestMassErase();
    HAL_Delay(1000);
#endif
}