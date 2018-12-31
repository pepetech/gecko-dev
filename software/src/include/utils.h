#ifndef __UTILS_H__
#define __UTILS_H__

#include <em_device.h>

#define IRAM_TEXT __attribute__ ((section(".iram0.text")))

#define REG_DISCARD(reg) __asm__ volatile ("" : : "r" (*(volatile uint32_t *)(reg)))

#define IS_VALID_APP(addr) ((*(volatile uint32_t *)(addr) & 0xFFFF0000) == SRAM_BASE)

#define IRQ_GROUP_BITS          3
#define IRQ_SUB_GROUP_BITS      1
#define IRQ_SET_PRIO(i, g, s)   NVIC->IP[i] = ((((g) & ((1 << IRQ_GROUP_BITS) - 1)) << IRQ_SUB_GROUP_BITS) | ((s) & ((1 << IRQ_SUB_GROUP_BITS) - 1))) << (8 - __NVIC_PRIO_BITS)
#define IRQ_GET_PRIO(i)         (NVIC->IP[i] >> (8 - __NVIC_PRIO_BITS))
#define IRQ_ENABLE(i)           NVIC->ISER[i >> 5] = 1 << (i & 0x1F)
#define IRQ_DISABLE(i)          NVIC->ICER[i >> 5] = 1 << (i & 0x1F)
#define IRQ_STATUS(i)           (NVIC->ISER[i >> 5] & (1 << (i & 0x1F)))

#define BIT(x) (1 << (x))

#endif
