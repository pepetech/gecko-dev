#include <em_device.h>
#include <stdlib.h>
#include <math.h>
#include "debug_macros.h"
#include "utils.h"
#include "atomic.h"
#include "systick.h"
#include "emu.h"
#include "cmu.h"
#include "dbg.h"
#include "msc.h"
#include "crypto.h"
#include "crc.h"
#include "trng.h"
#include "rtcc.h"
#include "adc.h"
#include "qspi.h"
#include "i2c.h"

// Structs

// Forward declarations
static void reset() __attribute__((noreturn));
static void sleep();

static uint32_t get_free_ram();

void get_device_name(char *pszDeviceName, uint32_t ulDeviceNameSize);
static uint16_t get_device_revision();

static void gpio_init();

// Variables

// ISRs

// Functions
void reset()
{
    SCB->AIRCR = 0x05FA0000 | _VAL2FLD(SCB_AIRCR_SYSRESETREQ, 1);

    while(1);
}
void sleep()
{
    rtcc_set_alarm(rtcc_get_time() + 5);

    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk; // Configure Deep Sleep (EM2/3)

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        IRQ_CLEAR(RTCC_IRQn);

        __DMB(); // Wait for all memory transactions to finish before memory access
        __DSB(); // Wait for all memory transactions to finish before executing instructions
        __ISB(); // Wait for all memory transactions to finish before fetching instructions
        __SEV(); // Set the event flag to ensure the next WFE will be a NOP
        __WFE(); // NOP and clear the event flag
        __WFE(); // Wait for event
        __NOP(); // Prevent debugger crashes

        cmu_init();
        cmu_update_clocks();
    }
}

uint32_t get_free_ram()
{
    void *pCurrentHeap = malloc(1);

    uint32_t ulFreeRAM = (uint32_t)__get_MSP() - (uint32_t)pCurrentHeap;

    free(pCurrentHeap);

    return ulFreeRAM;
}

void get_device_name(char *pszDeviceName, uint32_t ulDeviceNameSize)
{
    uint8_t ubFamily = (DEVINFO->PART & _DEVINFO_PART_DEVICE_FAMILY_MASK) >> _DEVINFO_PART_DEVICE_FAMILY_SHIFT;
    const char* szFamily = "?";

    if(ubFamily == 0x10)
        szFamily = "EFR32MG1P";
    else if(ubFamily == 0x11)
        szFamily = "EFR32MG1B";
    else if(ubFamily == 0x12)
        szFamily = "EFR32MG1V";
    else if(ubFamily == 0x13)
        szFamily = "EFR32BG1P";
    else if(ubFamily == 0x14)
        szFamily = "EFR32BG1B";
    else if(ubFamily == 0x15)
        szFamily = "EFR32BG1V";
    else if(ubFamily == 0x19)
        szFamily = "EFR32FG1P";
    else if(ubFamily == 0x1A)
        szFamily = "EFR32FG1B";
    else if(ubFamily == 0x1B)
        szFamily = "EFR32FG1V";
    else if(ubFamily == 0x1C)
        szFamily = "EFR32MG12P";
    else if(ubFamily == 0x1D)
        szFamily = "EFR32MG12B";
    else if(ubFamily == 0x1E)
        szFamily = "EFR32MG12V";
    else if(ubFamily == 0x1F)
        szFamily = "EFR32BG12P";
    else if(ubFamily == 0x20)
        szFamily = "EFR32BG12B";
    else if(ubFamily == 0x21)
        szFamily = "EFR32BG12V";
    else if(ubFamily == 0x25)
        szFamily = "EFR32FG12P";
    else if(ubFamily == 0x26)
        szFamily = "EFR32FG12B";
    else if(ubFamily == 0x27)
        szFamily = "EFR32FG12V";
    else if(ubFamily == 0x28)
        szFamily = "EFR32MG13P";
    else if(ubFamily == 0x29)
        szFamily = "EFR32MG13B";
    else if(ubFamily == 0x2A)
        szFamily = "EFR32MG13V";
    else if(ubFamily == 0x2B)
        szFamily = "EFR32BG13P";
    else if(ubFamily == 0x2C)
        szFamily = "EFR32BG13B";
    else if(ubFamily == 0x2D)
        szFamily = "EFR32BG13V";
    else if(ubFamily == 0x2E)
        szFamily = "EFR32ZG13P";
    else if(ubFamily == 0x31)
        szFamily = "EFR32FG13P";
    else if(ubFamily == 0x32)
        szFamily = "EFR32FG13B";
    else if(ubFamily == 0x33)
        szFamily = "EFR32FG13V";
    else if(ubFamily == 0x34)
        szFamily = "EFR32MG14P";
    else if(ubFamily == 0x35)
        szFamily = "EFR32MG14B";
    else if(ubFamily == 0x36)
        szFamily = "EFR32MG14V";
    else if(ubFamily == 0x37)
        szFamily = "EFR32BG14P";
    else if(ubFamily == 0x38)
        szFamily = "EFR32BG14B";
    else if(ubFamily == 0x39)
        szFamily = "EFR32BG14V";
    else if(ubFamily == 0x3A)
        szFamily = "EFR32ZG14P";
    else if(ubFamily == 0x3D)
        szFamily = "EFR32FG14P";
    else if(ubFamily == 0x3E)
        szFamily = "EFR32FG14B";
    else if(ubFamily == 0x3F)
        szFamily = "EFR32FG14V";
    else if(ubFamily == 0x47)
        szFamily = "EFM32G";
    else if(ubFamily == 0x47)
        szFamily = "G";
    else if(ubFamily == 0x48)
        szFamily = "EFM32GG";
    else if(ubFamily == 0x48)
        szFamily = "GG";
    else if(ubFamily == 0x49)
        szFamily = "TG";
    else if(ubFamily == 0x49)
        szFamily = "EFM32TG";
    else if(ubFamily == 0x4A)
        szFamily = "EFM32LG";
    else if(ubFamily == 0x4A)
        szFamily = "LG";
    else if(ubFamily == 0x4B)
        szFamily = "EFM32WG";
    else if(ubFamily == 0x4B)
        szFamily = "WG";
    else if(ubFamily == 0x4C)
        szFamily = "ZG";
    else if(ubFamily == 0x4C)
        szFamily = "EFM32ZG";
    else if(ubFamily == 0x4D)
        szFamily = "HG";
    else if(ubFamily == 0x4D)
        szFamily = "EFM32HG";
    else if(ubFamily == 0x51)
        szFamily = "EFM32PG1B";
    else if(ubFamily == 0x53)
        szFamily = "EFM32JG1B";
    else if(ubFamily == 0x55)
        szFamily = "EFM32PG12B";
    else if(ubFamily == 0x57)
        szFamily = "EFM32JG12B";
    else if(ubFamily == 0x64)
        szFamily = "EFM32GG11B";
    else if(ubFamily == 0x67)
        szFamily = "EFM32TG11B";
    else if(ubFamily == 0x6A)
        szFamily = "EFM32GG12B";
    else if(ubFamily == 0x78)
        szFamily = "EZR32LG";
    else if(ubFamily == 0x79)
        szFamily = "EZR32WG";
    else if(ubFamily == 0x7A)
        szFamily = "EZR32HG";
    
    uint8_t ubPackage = (DEVINFO->MEMINFO & _DEVINFO_MEMINFO_PKGTYPE_MASK) >> _DEVINFO_MEMINFO_PKGTYPE_SHIFT;
    char cPackage = '?';
    
    if(ubPackage == 74)
        cPackage = '?';
    else if(ubPackage == 76)
        cPackage = 'L';
    else if(ubPackage == 77)
        cPackage = 'M';
    else if(ubPackage == 81)
        cPackage = 'Q';

    uint8_t ubTempGrade = (DEVINFO->MEMINFO & _DEVINFO_MEMINFO_TEMPGRADE_MASK) >> _DEVINFO_MEMINFO_TEMPGRADE_SHIFT;
    char cTempGrade = '?';

    if(ubTempGrade == 0)
        cTempGrade = 'G';
    else if(ubTempGrade == 1)
        cTempGrade = 'I';
    else if(ubTempGrade == 2)
        cTempGrade = '?';
    else if(ubTempGrade == 3)
        cTempGrade = '?';

    uint16_t usPartNumber = (DEVINFO->PART & _DEVINFO_PART_DEVICE_NUMBER_MASK) >> _DEVINFO_PART_DEVICE_NUMBER_SHIFT;
    uint8_t ubPinCount = (DEVINFO->MEMINFO & _DEVINFO_MEMINFO_PINCOUNT_MASK) >> _DEVINFO_MEMINFO_PINCOUNT_SHIFT;

    snprintf(pszDeviceName, ulDeviceNameSize, "%s%huF%hu%c%c%hhu", szFamily, usPartNumber, FLASH_SIZE >> 10, cTempGrade, cPackage, ubPinCount);
}
uint16_t get_device_revision()
{
    uint16_t usRevision;

    /* CHIP MAJOR bit [3:0]. */
    usRevision = ((ROMTABLE->PID0 & _ROMTABLE_PID0_REVMAJOR_MASK) >> _ROMTABLE_PID0_REVMAJOR_SHIFT) << 8;
    /* CHIP MINOR bit [7:4]. */
    usRevision |= ((ROMTABLE->PID2 & _ROMTABLE_PID2_REVMINORMSB_MASK) >> _ROMTABLE_PID2_REVMINORMSB_SHIFT) << 4;
    /* CHIP MINOR bit [3:0]. */
    usRevision |= (ROMTABLE->PID3 & _ROMTABLE_PID3_REVMINORLSB_MASK) >> _ROMTABLE_PID3_REVMINORLSB_SHIFT;

    return usRevision;
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
                      | GPIO_P_MODEH_MODE11_WIREDANDPULLUPFILTER // I2C1_SDA - Location 1
                      | GPIO_P_MODEH_MODE12_WIREDANDPULLUPFILTER // I2C1_SCL - Location 1
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
                      | GPIO_P_MODEH_MODE10_DISABLED
                      | GPIO_P_MODEH_MODE11_DISABLED
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
                      | GPIO_P_MODEH_MODE9_PUSHPULL  // QSPI0_DQ0 - Location 0
                      | GPIO_P_MODEH_MODE10_PUSHPULL // QSPI0_DQ1 - Location 0
                      | GPIO_P_MODEH_MODE11_PUSHPULL // QSPI0_DQ2 - Location 0
                      | GPIO_P_MODEH_MODE12_PUSHPULL // QSPI0_DQ3 - Location 0
                      | GPIO_P_MODEH_MODE13_DISABLED
                      | GPIO_P_MODEH_MODE14_DISABLED
                      | GPIO_P_MODEH_MODE15_DISABLED;
    GPIO->P[3].DOUT   = 0;
    GPIO->P[3].OVTDIS = 0;

    // Port E
    GPIO->P[4].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[4].MODEL  = GPIO_P_MODEL_MODE0_DISABLED
                      | GPIO_P_MODEL_MODE1_DISABLED
                      | GPIO_P_MODEL_MODE2_DISABLED
                      | GPIO_P_MODEL_MODE3_DISABLED
                      | GPIO_P_MODEL_MODE4_DISABLED
                      | GPIO_P_MODEL_MODE5_DISABLED
                      | GPIO_P_MODEL_MODE6_DISABLED
                      | GPIO_P_MODEL_MODE7_DISABLED;
    GPIO->P[4].MODEH  = GPIO_P_MODEH_MODE8_DISABLED
                      | GPIO_P_MODEH_MODE9_DISABLED
                      | GPIO_P_MODEH_MODE10_DISABLED
                      | GPIO_P_MODEH_MODE11_DISABLED
                      | GPIO_P_MODEH_MODE12_DISABLED
                      | GPIO_P_MODEH_MODE13_DISABLED
                      | GPIO_P_MODEH_MODE14_DISABLED
                      | GPIO_P_MODEH_MODE15_DISABLED;
    GPIO->P[4].DOUT   = 0;
    GPIO->P[4].OVTDIS = 0;

    // Port F
    GPIO->P[5].CTRL   = GPIO_P_CTRL_DRIVESTRENGTHALT_STRONG | (5 << _GPIO_P_CTRL_SLEWRATEALT_SHIFT)
                      | GPIO_P_CTRL_DRIVESTRENGTH_STRONG | (5 << _GPIO_P_CTRL_SLEWRATE_SHIFT);
    GPIO->P[5].MODEL  = GPIO_P_MODEL_MODE0_PUSHPULL  // DBG_SWCLK - Location 0
                      | GPIO_P_MODEL_MODE1_PUSHPULL  // DBG_SWDIO - Location 0
                      | GPIO_P_MODEL_MODE2_PUSHPULL  // DBG_SWO - Location 0
                      | GPIO_P_MODEL_MODE3_DISABLED
                      | GPIO_P_MODEL_MODE4_DISABLED
                      | GPIO_P_MODEL_MODE5_DISABLED
                      | GPIO_P_MODEL_MODE6_PUSHPULL  // QSPI0_SCLK - Location 0
                      | GPIO_P_MODEL_MODE7_PUSHPULL; // QSPI0_CS0 - Location 0
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
}

int init()
{
    emu_init(); // Init EMU

    cmu_hfxo_startup_calib(0x200, 0x087); // Config HFXO Startup for 1280 uA, 20.04 pF
    cmu_hfxo_steady_calib(0x006, 0x087); // Config HFXO Steady state for 12 uA, 20.04 pF

    cmu_init(); // Inic Clocks

    cmu_ushfrco_calib(1, USHFRCO_CALIB_8M, 8000000); // Enable and calibrate USHFRCO for 8 MHz
    cmu_auxhfrco_calib(1, AUXHFRCO_CALIB_32M, 32000000); // Enable and calibrate AUXHFRCO for 32 MHz

    cmu_update_clocks(); // Update Clocks

    dbg_init(); // Init Debug module
    dbg_swo_config(BIT(0) | BIT(1), 2000000); // Init SWO channels 0 and 1 at 2 MHz

    msc_init(); // Init Flash, RAM and caches

    systick_init(); // Init system tick

    gpio_init(); // Init GPIOs
    rtcc_init(); // Init RTCC
    crypto_init(); // Init Crypto engine
    crc_init(); // Init CRC calculation unit
    adc_init(); // Init ADCs
    qspi_init(); // Init QSPI memory
    
    float fAVDDHighThresh, fAVDDLowThresh;
    float fDVDDHighThresh, fDVDDLowThresh;
    float fIOVDDHighThresh, fIOVDDLowThresh;

    emu_vmon_avdd_config(1, 3.1f, &fAVDDLowThresh, 3.22f, &fAVDDHighThresh); // Enable AVDD monitor
    emu_vmon_dvdd_config(1, 2.5f, &fDVDDLowThresh); // Enable DVDD monitor
    emu_vmon_iovdd_config(1, 3.15f, &fIOVDDLowThresh); // Enable IOVDD monitor

    fDVDDHighThresh = fDVDDLowThresh + 0.026f; // Hysteresis from datasheet
    fIOVDDHighThresh = fIOVDDLowThresh + 0.026f; // Hysteresis from datasheet

    i2c1_init(I2C_NORMAL, 1, 1); // Init I2C1 at 100 kHz on location 1

    char szDeviceName[32];

    get_device_name(szDeviceName, 32);

    DBGPRINTLN_CTX("Device: %s", szDeviceName);
    DBGPRINTLN_CTX("Device Revision: 0x%04X", get_device_revision());
    DBGPRINTLN_CTX("Calibration temperature: %hhu C", (DEVINFO->CAL & _DEVINFO_CAL_TEMP_MASK) >> _DEVINFO_CAL_TEMP_SHIFT);
    DBGPRINTLN_CTX("Flash Size: %hu kB", FLASH_SIZE >> 10);
    DBGPRINTLN_CTX("RAM Size: %hu kB", SRAM_SIZE >> 10);
    DBGPRINTLN_CTX("Free RAM: %lu B", get_free_ram());
    DBGPRINTLN_CTX("Unique ID: %08X-%08X", DEVINFO->UNIQUEH, DEVINFO->UNIQUEL);

    DBGPRINTLN_CTX("CMU - HFXO Clock: %.1f MHz!", (float)HFXO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - HFRCO Clock: %.1f MHz!", (float)HFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - USHFRCO Clock: %.1f MHz!", (float)USHFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - AUXHFRCO Clock: %.1f MHz!", (float)AUXHFRCO_VALUE / 1000000);
    DBGPRINTLN_CTX("CMU - LFXO Clock: %.3f kHz!", (float)LFXO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - LFRCO Clock: %.3f kHz!", (float)LFRCO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - ULFRCO Clock: %.3f kHz!", (float)ULFRCO_VALUE / 1000);
    DBGPRINTLN_CTX("CMU - HFSRC Clock: %.1f MHz!", (float)HFSRC_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HF Clock: %.1f MHz!", (float)HF_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFBUS Clock: %.1f MHz!", (float)HFBUS_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFCORE Clock: %.1f MHz!", (float)HFCORE_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFEXP Clock: %.1f MHz!", (float)HFEXP_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPER Clock: %.1f MHz!", (float)HFPER_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPERB Clock: %.1f MHz!", (float)HFPERB_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFPERC Clock: %.1f MHz!", (float)HFPERC_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - HFLE Clock: %.1f MHz!", (float)HFLE_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - QSPI Clock: %.1f MHz!", (float)QSPI_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - SDIO Clock: %.1f MHz!", (float)SDIO_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - USB Clock: %.1f MHz!", (float)USB_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - ADC0 Clock: %.1f MHz!", (float)ADC0_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - ADC1 Clock: %.1f MHz!", (float)ADC1_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - DBG Clock: %.1f MHz!", (float)DBG_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - AUX Clock: %.1f MHz!", (float)AUX_CLOCK_FREQ / 1000000);
    DBGPRINTLN_CTX("CMU - LFA Clock: %.3f kHz!", (float)LFA_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LESENSE Clock: %.3f kHz!", (float)LESENSE_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - RTC Clock: %.3f kHz!", (float)RTC_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LCD Clock: %.3f kHz!", (float)LCD_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LETIMER0 Clock: %.3f kHz!", (float)LETIMER0_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LETIMER1 Clock: %.3f kHz!", (float)LETIMER1_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFB Clock: %.3f kHz!", (float)LFB_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LEUART0 Clock: %.3f kHz!", (float)LEUART0_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LEUART1 Clock: %.3f kHz!", (float)LEUART1_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - SYSTICK Clock: %.3f kHz!", (float)SYSTICK_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - CSEN Clock: %.3f kHz!", (float)CSEN_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFC Clock: %.3f kHz!", (float)LFC_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - LFE Clock: %.3f kHz!", (float)LFE_CLOCK_FREQ / 1000);
    DBGPRINTLN_CTX("CMU - RTCC Clock: %.3f kHz!", (float)RTCC_CLOCK_FREQ / 1000);

    DBGPRINTLN_CTX("EMU - AVDD Fall Threshold: %.2f mV!", fAVDDLowThresh * 1000);
    DBGPRINTLN_CTX("EMU - AVDD Rise Threshold: %.2f mV!", fAVDDHighThresh * 1000);
    DBGPRINTLN_CTX("EMU - AVDD Voltage: %.2f mV", adc_get_avdd());
    DBGPRINTLN_CTX("EMU - AVDD Status: %s", g_ubAVDDLow ? "LOW" : "OK");
    DBGPRINTLN_CTX("EMU - DVDD Fall Threshold: %.2f mV!", fDVDDLowThresh * 1000);
    DBGPRINTLN_CTX("EMU - DVDD Rise Threshold: %.2f mV!", fDVDDHighThresh * 1000);
    DBGPRINTLN_CTX("EMU - DVDD Voltage: %.2f mV", adc_get_dvdd());
    DBGPRINTLN_CTX("EMU - DVDD Status: %s", g_ubDVDDLow ? "LOW" : "OK");
    DBGPRINTLN_CTX("EMU - IOVDD Fall Threshold: %.2f mV!", fIOVDDLowThresh * 1000);
    DBGPRINTLN_CTX("EMU - IOVDD Rise Threshold: %.2f mV!", fIOVDDHighThresh * 1000);
    DBGPRINTLN_CTX("EMU - IOVDD Voltage: %.2f mV", adc_get_iovdd());
    DBGPRINTLN_CTX("EMU - IOVDD Status: %s", g_ubIOVDDLow ? "LOW" : "OK");
    DBGPRINTLN_CTX("EMU - Core Voltage: %.2f mV", adc_get_corevdd());

    delay_ms(100);

    DBGPRINTLN_CTX("Scanning I2C bus 1...");

    for(uint8_t a = 0x08; a < 0x78; a++)
    {
        if(i2c1_write(a, 0, 0, I2C_STOP))
            DBGPRINTLN_CTX("  Address 0x%02X ACKed!", a);
    }

    return 0;
}
int main()
{
    i2c1_write_byte(0x76, 0xD0, I2C_RESTART);
    DBGPRINTLN_CTX("BME ID %02X", i2c1_read_byte(0x76, I2C_STOP));

    // Internal flash test
    DBGPRINTLN_CTX("Initial calibration dump:");
    
    for(init_calib_t *psCalibTbl = g_psInitCalibrationTable; psCalibTbl->pulRegister; psCalibTbl++)
        DBGPRINTLN_CTX("  0x%08X -> 0x%08X", psCalibTbl->ulInitialCalibration, psCalibTbl->pulRegister);

    /*
    DBGPRINTLN_CTX("Boot lock word: %08X", g_psLockBits->CLW[0]);
    DBGPRINTLN_CTX("User lock word: %08X", g_psLockBits->ULW);
    DBGPRINTLN_CTX("Mass lock word: %08X", g_psLockBits->MLW);
    msc_flash_word_write((uint32_t)&(g_psLockBits->MLW), 0xFFFFFFFD);
    DBGPRINTLN_CTX("Mass lock word: %08X", g_psLockBits->MLW);

    DBGPRINTLN_CTX("0x000FFFFC: %08X", *(volatile uint32_t *)0x000FFFFC);
    DBGPRINTLN_CTX("0x00100000: %08X", *(volatile uint32_t *)0x00100000);
    msc_flash_word_write(0x000FFFFC, 0x12344321);
    msc_flash_word_write(0x00100000, 0xABCDDCBA);
    DBGPRINTLN_CTX("0x000FFFFC: %08X", *(volatile uint32_t *)0x000FFFFC);
    DBGPRINTLN_CTX("0x00100000: %08X", *(volatile uint32_t *)0x00100000);
    msc_flash_unlock();
    MSC->WRITECMD = MSC_WRITECMD_ERASEMAIN1;
    msc_flash_lock();
    DBGPRINTLN_CTX("0x000FFFFC: %08X", *(volatile uint32_t *)0x000FFFFC);
    DBGPRINTLN_CTX("0x00100000: %08X", *(volatile uint32_t *)0x00100000);
    */

    // QSPI
    DBGPRINTLN_CTX("Flash Part ID: %06X", qspi_flash_read_jedec_id());

    uint8_t ubFlashUID[8];

    qspi_flash_read_security(0x0000, ubFlashUID, 8);

    DBGPRINTLN_CTX("Flash ID: %02X%02X%02X%02X%02X%02X%02X%02X", ubFlashUID[0], ubFlashUID[1], ubFlashUID[2], ubFlashUID[3], ubFlashUID[4], ubFlashUID[5], ubFlashUID[6], ubFlashUID[7]);

    //qspi_flash_chip_erase();

    //uint8_t rd[16];

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00008000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);
    
    //DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000000);
    //*(volatile uint32_t *)0xC0000000 = 0xABCDEF12;

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00000000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);

    //uint32_t wr = 0xA2B3C4D5;
    //qspi_flash_busy_wait();
    //qspi_flash_write_enable();
    //qspi_flash_cmd(QSPI_FLASH_CMD_WRITE, 0x00000004, 3, 0, 0, (uint8_t*)&wr, 4, NULL, 0);
    //qspi_flash_busy_wait();

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00000000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);

    //*(volatile uint32_t *)0xC0000000 = 0x12AB34CD;

    //////// Test for page wrapping (write beyond page boundary)
    
    /*
    for(uint8_t i = 0; i <= 64; i++)
        *(volatile uint32_t *)(0xC0000000 + i * 4) = 0x0123ABCD;

    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000000); // CD
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000001); // AB
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000002); // 23
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC0000003); // 01
    DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000000); // 0123ABCD
    DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000010); // 0123ABCD
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FC); // CD
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FD); // AB
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FE); // 23
    DBGPRINTLN_CTX("Flash RD: %02X", *(volatile uint8_t *)0xC00000FF); // 01
    DBGPRINTLN_CTX("Flash RD: %08X", *(volatile uint32_t *)0xC0000100); // 0123ABCD
    */

    //////// Test for code copy to QSPI flash
    
    /*
    for(uint32_t i = 0; i < bin_v1_test_bin_qspi_len / 4; i++)
        *(volatile uint32_t *)(0x04000000 + i * 4) = *(uint32_t *)(bin_v1_test_bin_qspi + i * 4);
    
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000000);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000001);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000002);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000003);

    DBGPRINTLN_CTX("QSPI Dest %08X", get_family_name);
    DBGPRINTLN_CTX("Device: %s%hu", get_family_name((DEVINFO->PART & _DEVINFO_PART_DEVICE_FAMILY_MASK) >> _DEVINFO_PART_DEVICE_FAMILY_SHIFT), (DEVINFO->PART & _DEVINFO_PART_DEVICE_NUMBER_MASK) >> _DEVINFO_PART_DEVICE_NUMBER_SHIFT);
    */

    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000000);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000001);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000002);
    DBGPRINTLN_CTX("QSPI RD: %02X", *(volatile uint8_t *)0xC0000003);
    DBGPRINTLN_CTX("Boot RD: %02X", *(volatile uint8_t *)0x0FE10000);
    DBGPRINTLN_CTX("Data RD: %02X", *(volatile uint8_t *)0x0FE00000);

    //qspi_flash_cmd(QSPI_FLASH_CMD_READ_FAST, 0x00000000, 3, 0, 8, NULL, 0, rd, 10);
    //DBGPRINTLN_CTX("Flash RD C: %02X%02X%02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X%02X%02X", rd[0], rd[1], rd[2], rd[3], rd[4], rd[5], rd[6], rd[7], rd[8], rd[9], rd[10], rd[11], rd[12], rd[13], rd[14], rd[15]);

    //DBGPRINTLN_CTX("QSPI RD: %08X", *(volatile uint32_t *)0xC0000000);
    //DBGPRINTLN_CTX("QSPI RD: %08X", *(volatile uint32_t *)0xC0000004);

    while(1)
    {
        GPIO->P[0].DOUT ^= BIT(0);
        
        delay_ms(500);
        
        DBGPRINTLN_CTX("ADC Temp: %.2f", adc_get_temperature());
        DBGPRINTLN_CTX("EMU Temp: %.2f", emu_get_temperature());

        DBGPRINTLN_CTX("HFXO Startup: %.2f pF", cmu_hfxo_get_startup_cap());
        DBGPRINTLN_CTX("HFXO Startup: %.2f uA", cmu_hfxo_get_startup_current());
        DBGPRINTLN_CTX("HFXO Steady: %.2f pF", cmu_hfxo_get_steady_cap());
        DBGPRINTLN_CTX("HFXO Steady: %.2f uA", cmu_hfxo_get_steady_current());
        DBGPRINTLN_CTX("HFXO PMA [%03X]: %.2f uA", cmu_hfxo_get_pma_ibtrim(), cmu_hfxo_get_pma_current());
        DBGPRINTLN_CTX("HFXO PDA [%03X]: %.2f uA", cmu_hfxo_get_pda_ibtrim(1), cmu_hfxo_get_pda_current(0));
       
        //sleep();

        DBGPRINTLN_CTX("RTCC Time: %lu", rtcc_get_time());

        DBGPRINTLN_CTX("Big fag does not need debug uart anymore.");
    }

    return 0;
}