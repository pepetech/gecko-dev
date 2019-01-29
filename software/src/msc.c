#include "msc.h"

void msc_init()
{
    MSC->CTRL |= MSC_CTRL_CLKDISFAULTEN | MSC_CTRL_ADDRFAULTEN | MSC_CTRL_IFCREADCLEAR;
    MSC->BOOTLOADERCTRL = MSC_BOOTLOADERCTRL_BLWDIS;
    MSC->READCTRL |= MSC_READCTRL_SCBTP;
    MSC->WRITECTRL = MSC_WRITECTRL_RWWEN;
    MSC->CACHECMD = MSC_CACHECMD_INVCACHE;
}
void msc_config_waitstates(uint32_t ulFrequency)
{
    if(ulFrequency <= 18000000)
        MSC->READCTRL = (MSC->READCTRL & ~_MSC_READCTRL_MODE_MASK) | MSC_READCTRL_MODE_WS0;
    else if(ulFrequency <= 36000000)
        MSC->READCTRL = (MSC->READCTRL & ~_MSC_READCTRL_MODE_MASK) | MSC_READCTRL_MODE_WS1;
    else if(ulFrequency <= 54000000)
        MSC->READCTRL = (MSC->READCTRL & ~_MSC_READCTRL_MODE_MASK) | MSC_READCTRL_MODE_WS2;
    else
        MSC->READCTRL = (MSC->READCTRL & ~_MSC_READCTRL_MODE_MASK) | MSC_READCTRL_MODE_WS3;

    if(ulFrequency <= 50000000)
        MSC->CTRL &= ~MSC_CTRL_WAITMODE_WS1;
    else
        MSC->CTRL |= MSC_CTRL_WAITMODE_WS1;

    if(ulFrequency <= 38000000)
        MSC->RAMCTRL &= ~(MSC_RAMCTRL_RAM2WSEN | MSC_RAMCTRL_RAM2PREFETCHEN | MSC_RAMCTRL_RAM1WSEN | MSC_RAMCTRL_RAM1PREFETCHEN | MSC_RAMCTRL_RAMWSEN | MSC_RAMCTRL_RAMPREFETCHEN);
    else
        MSC->RAMCTRL |= MSC_RAMCTRL_RAM2WSEN | MSC_RAMCTRL_RAM2PREFETCHEN | MSC_RAMCTRL_RAM1WSEN | MSC_RAMCTRL_RAM1PREFETCHEN | MSC_RAMCTRL_RAMWSEN | MSC_RAMCTRL_RAMPREFETCHEN;
}
void msc_flash_lock()
{

}
void msc_flash_unlock()
{

}
void msc_flash_page_erase(uint32_t ulAddress)
{
    if(ulAddress < FLASH_BASE || ulAddress > FLASH_MEM_END)
        return;
    
}
void msc_flash_page_write(uint32_t ulAddress, uint8_t *pubData, uint32_t ulSize)
{
    if(ulAddress < FLASH_BASE || ulAddress > FLASH_MEM_END)
        return;
}
