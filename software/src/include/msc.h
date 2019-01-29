#ifndef __MSC_H__
#define __MSC_H__

#include <em_device.h>

#define FLASH_PAGE_COUNT    (FLASH_SIZE / FLASH_PAGE_SIZE)

void msc_init();
void msc_config_waitstates(uint32_t ulFrequency);
void msc_flash_lock();
void msc_flash_unlock();
void msc_flash_page_erase(uint32_t ulAddress);
void msc_flash_page_write(uint32_t ulAddress, uint8_t *pubData, uint32_t ulSize);

#endif
