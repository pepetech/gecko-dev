#include "gpio.h"

static void gpio_isr(uint32_t ulFlags)
{

}
void _gpio_even_isr()
{
    uint32_t ulFlags = GPIO->IF;

    gpio_isr(ulFlags & 0x55555555);

    GPIO->IFC = 0x55555555; // Clear all even flags
}
void _gpio_odd_isr()
{
    uint32_t ulFlags = GPIO->IF;

    gpio_isr(ulFlags & 0xAAAAAAAA);

    GPIO->IFC = 0xAAAAAAAA; // Clear all odd flags
}

void gpio_init()
{
    CMU->HFBUSCLKEN0 |= CMU_HFBUSCLKEN0_GPIO;

    // Port A
    GPIO->P[0].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[0].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL  // GPIO - LED
                      | GPIO_P_MODEL_MODE1_DISABLED
                      | GPIO_P_MODEL_MODE2_DISABLED
                      | GPIO_P_MODEL_MODE3_DISABLED
                      | GPIO_P_MODEL_MODE4_DISABLED
                      | GPIO_P_MODEL_MODE5_DISABLED
                      | GPIO_P_MODEL_MODE6_DISABLED
                      | GPIO_P_MODEL_MODE7_DISABLED;
    GPIO->P[0].MODEH  = GPIO_P_MODEH_MODE8_DISABLED
                      | GPIO_P_MODEH_MODE9_DISABLED
                      | GPIO_P_MODEH_MODE10_DISABLED
                      | GPIO_P_MODEH_MODE11_DISABLED
                      | GPIO_P_MODEH_MODE12_DISABLED
                      | GPIO_P_MODEH_MODE13_DISABLED
                      | GPIO_P_MODEH_MODE14_DISABLED
                      | GPIO_P_MODEH_MODE15_DISABLED;
    GPIO->P[0].DOUT   = 0;
    GPIO->P[0].OVTDIS = 0;

    // Port B
    GPIO->P[1].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (7 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (7 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[1].MODEL  = GPIO_P_MODEL_MODE0_DISABLED
                      | GPIO_P_MODEL_MODE1_DISABLED
                      | GPIO_P_MODEL_MODE2_DISABLED
                      | GPIO_P_MODEL_MODE3_DISABLED
                      | GPIO_P_MODEL_MODE4_DISABLED
                      | GPIO_P_MODEL_MODE5_DISABLED
                      | GPIO_P_MODEL_MODE6_DISABLED
                      | GPIO_P_MODEL_MODE7_DISABLED;
    GPIO->P[1].MODEH  = GPIO_P_MODEH_MODE8_DISABLED
                      | GPIO_P_MODEH_MODE9_DISABLED
                      | GPIO_P_MODEH_MODE10_DISABLED
                      | GPIO_P_MODEH_MODE11_DISABLED
                      | GPIO_P_MODEH_MODE12_DISABLED
                      | GPIO_P_MODEH_MODE13_DISABLED
                      | GPIO_P_MODEH_MODE14_DISABLED
                      | GPIO_P_MODEH_MODE15_DISABLED;
    GPIO->P[1].DOUT   = BIT(11) | BIT(12);
    GPIO->P[1].OVTDIS = 0;

    // Port C
    GPIO->P[2].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[2].MODEL  = GPIO_P_MODEL_MODE0_DISABLED
                      | GPIO_P_MODEL_MODE1_DISABLED
                      | GPIO_P_MODEL_MODE2_DISABLED
                      | GPIO_P_MODEL_MODE3_DISABLED
                      | GPIO_P_MODEL_MODE4_DISABLED
                      | GPIO_P_MODEL_MODE5_DISABLED
                      | GPIO_P_MODEL_MODE6_DISABLED
                      | GPIO_P_MODEL_MODE7_DISABLED;
    GPIO->P[2].MODEH  = GPIO_P_MODEH_MODE8_DISABLED
                      | GPIO_P_MODEH_MODE9_DISABLED
                      | GPIO_P_MODEH_MODE10_WIREDANDPULLUPFILTER
                      | GPIO_P_MODEH_MODE11_WIREDANDPULLUPFILTER
                      | GPIO_P_MODEH_MODE12_DISABLED
                      | GPIO_P_MODEH_MODE13_DISABLED
                      | GPIO_P_MODEH_MODE14_DISABLED
                      | GPIO_P_MODEH_MODE15_DISABLED;
    GPIO->P[2].DOUT   = 0;
    GPIO->P[2].OVTDIS = 0;

    // Port D
    GPIO->P[3].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[3].MODEL  = GPIO_P_MODEL_MODE0_DISABLED
                      | GPIO_P_MODEL_MODE1_DISABLED
                      | GPIO_P_MODEL_MODE2_DISABLED
                      | GPIO_P_MODEL_MODE3_DISABLED
                      | GPIO_P_MODEL_MODE4_DISABLED
                      | GPIO_P_MODEL_MODE5_DISABLED
                      | GPIO_P_MODEL_MODE6_DISABLED
                      | GPIO_P_MODEL_MODE7_DISABLED;
    GPIO->P[3].MODEH  = GPIO_P_MODEH_MODE8_DISABLED
                      | GPIO_P_MODEH_MODE9_DISABLED
                      | GPIO_P_MODEH_MODE10_DISABLED
                      | GPIO_P_MODEH_MODE11_DISABLED
                      | GPIO_P_MODEH_MODE12_DISABLED
                      | GPIO_P_MODEH_MODE13_DISABLED
                      | GPIO_P_MODEH_MODE14_DISABLED
                      | GPIO_P_MODEH_MODE15_DISABLED;
    GPIO->P[3].DOUT   = 0;
    GPIO->P[3].OVTDIS = 0;

    // Port F
    GPIO->P[5].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[5].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL  // DBG_SWCLK - Location 0
                      | GPIO_P_MODEL_MODE1_PUSHPULL  // DBG_SWDIO - Location 0
                      | GPIO_P_MODEL_MODE2_PUSHPULL  // DBG_SWO - Location 0
                      | GPIO_P_MODEL_MODE3_DISABLED
                      | GPIO_P_MODEL_MODE4_DISABLED
                      | GPIO_P_MODEL_MODE5_DISABLED
                      | GPIO_P_MODEL_MODE6_DISABLED
                      | GPIO_P_MODEL_MODE7_DISABLED;
    GPIO->P[5].MODEH  = GPIO_P_MODEH_MODE8_DISABLED
                      | GPIO_P_MODEH_MODE9_DISABLED
                      | GPIO_P_MODEH_MODE10_DISABLED
                      | GPIO_P_MODEH_MODE11_DISABLED
                      | GPIO_P_MODEH_MODE12_DISABLED
                      | GPIO_P_MODEH_MODE13_DISABLED
                      | GPIO_P_MODEH_MODE14_DISABLED
                      | GPIO_P_MODEH_MODE15_DISABLED;
    GPIO->P[5].DOUT   = BIT(7);
    GPIO->P[5].OVTDIS = 0;

    // Debugger Route
    GPIO->ROUTEPEN &= ~(GPIO_ROUTEPEN_TDIPEN | GPIO_ROUTEPEN_TDOPEN); // Disable JTAG
    GPIO->ROUTEPEN |= GPIO_ROUTEPEN_SWVPEN; // Enable SWO
    GPIO->ROUTELOC0 = GPIO_ROUTELOC0_SWVLOC_LOC0; // SWO on PF2

    // External interrupts
    GPIO->EXTIPSELL = GPIO_EXTIPSELL_EXTIPSEL0_PORTA            // 
                    | GPIO_EXTIPSELL_EXTIPSEL1_PORTA            // 
                    | GPIO_EXTIPSELL_EXTIPSEL2_PORTA            // 
                    | GPIO_EXTIPSELL_EXTIPSEL3_PORTA            // 
                    | GPIO_EXTIPSELL_EXTIPSEL4_PORTA            // 
                    | GPIO_EXTIPSELL_EXTIPSEL5_PORTA            // 
                    | GPIO_EXTIPSELL_EXTIPSEL6_PORTA            // 
                    | GPIO_EXTIPSELL_EXTIPSEL7_PORTA;           // 
    GPIO->EXTIPSELH = GPIO_EXTIPSELH_EXTIPSEL8_PORTA            // 
                    | GPIO_EXTIPSELH_EXTIPSEL9_PORTA            // 
                    | GPIO_EXTIPSELH_EXTIPSEL10_PORTA           // 
                    | GPIO_EXTIPSELH_EXTIPSEL11_PORTA           // 
                    | GPIO_EXTIPSELH_EXTIPSEL12_PORTA           // 
                    | GPIO_EXTIPSELH_EXTIPSEL13_PORTA           // 
                    | GPIO_EXTIPSELH_EXTIPSEL14_PORTA           // 
                    | GPIO_EXTIPSELH_EXTIPSEL15_PORTA;          // 

    GPIO->EXTIPINSELL = GPIO_EXTIPINSELL_EXTIPINSEL0_PIN3       // 
                      | GPIO_EXTIPINSELL_EXTIPINSEL1_PIN1       // 
                      | GPIO_EXTIPINSELL_EXTIPINSEL2_PIN2       // 
                      | GPIO_EXTIPINSELL_EXTIPINSEL3_PIN3       // 
                      | GPIO_EXTIPINSELL_EXTIPINSEL4_PIN6       // 
                      | GPIO_EXTIPINSELL_EXTIPINSEL5_PIN7       // 
                      | GPIO_EXTIPINSELL_EXTIPINSEL6_PIN4       // 
                      | GPIO_EXTIPINSELL_EXTIPINSEL7_PIN7;      // 
    GPIO->EXTIPINSELH = GPIO_EXTIPINSELH_EXTIPINSEL8_PIN8       // 
                      | GPIO_EXTIPINSELH_EXTIPINSEL9_PIN9       // 
                      | GPIO_EXTIPINSELH_EXTIPINSEL10_PIN11     // 
                      | GPIO_EXTIPINSELH_EXTIPINSEL11_PIN8      // 
                      | GPIO_EXTIPINSELH_EXTIPINSEL12_PIN13     // 
                      | GPIO_EXTIPINSELH_EXTIPINSEL13_PIN15     // 
                      | GPIO_EXTIPINSELH_EXTIPINSEL14_PIN12     // 
                      | GPIO_EXTIPINSELH_EXTIPINSEL15_PIN12;    // 

    GPIO->EXTIRISE = 0; // 
    GPIO->EXTIFALL = 0; // 

    GPIO->IFC = _GPIO_IFC_MASK; // Clear pending IRQs
    IRQ_CLEAR(GPIO_EVEN_IRQn); // Clear pending vector
    IRQ_CLEAR(GPIO_ODD_IRQn); // Clear pending vector
    IRQ_SET_PRIO(GPIO_EVEN_IRQn, 0, 0); // Set priority 0,0 (max)
    IRQ_SET_PRIO(GPIO_ODD_IRQn, 0, 0); // Set priority 0,0 (max)
    IRQ_ENABLE(GPIO_EVEN_IRQn); // Enable vector
    IRQ_ENABLE(GPIO_ODD_IRQn); // Enable vector
    GPIO->IEN = 0; // Enable interrupts
}