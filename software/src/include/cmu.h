#ifndef __CMU_H__
#define __CMU_H__

#include <em_device.h>
#include "msc.h"

extern uint32_t HFRCO_VALUE;
extern uint32_t USHFRCO_VALUE;
extern uint32_t AUXHFRCO_VALUE;
extern uint32_t LFRCO_VALUE;
extern uint32_t ULFRCO_VALUE;

extern uint32_t HFSRC_CLOCK_FREQ;
extern uint32_t HF_CLOCK_FREQ;
extern uint32_t HFBUS_CLOCK_FREQ;
extern uint32_t HFCORE_CLOCK_FREQ;
extern uint32_t HFEXP_CLOCK_FREQ;
extern uint32_t HFPER_CLOCK_FREQ;
extern uint32_t HFPERB_CLOCK_FREQ;
extern uint32_t HFPERC_CLOCK_FREQ;
extern uint32_t HFLE_CLOCK_FREQ;
extern uint32_t QSPI_CLOCK_FREQ;
extern uint32_t SDIO_CLOCK_FREQ;
extern uint32_t USB_CLOCK_FREQ;
extern uint32_t ADC0_CLOCK_FREQ;
extern uint32_t ADC1_CLOCK_FREQ;
extern uint32_t DBG_CLOCK_FREQ;
extern uint32_t AUX_CLOCK_FREQ;
extern uint32_t LFA_CLOCK_FREQ;
extern uint32_t LESENSE_CLOCK_FREQ;
extern uint32_t RTC_CLOCK_FREQ;
extern uint32_t LCD_CLOCK_FREQ;
extern uint32_t LETIMER0_CLOCK_FREQ;
extern uint32_t LETIMER1_CLOCK_FREQ;
extern uint32_t LFB_CLOCK_FREQ;
extern uint32_t LEUART0_CLOCK_FREQ;
extern uint32_t LEUART1_CLOCK_FREQ;
extern uint32_t SYSTICK_CLOCK_FREQ;
extern uint32_t CSEN_CLOCK_FREQ;
extern uint32_t LFC_CLOCK_FREQ;
extern uint32_t LFE_CLOCK_FREQ;
extern uint32_t RTCC_CLOCK_FREQ;

#define HFRCO_CALIB_1M      (DEVINFO->HFRCOCAL0 | CMU_HFRCOCTRL_CLKDIV_DIV4)
#define HFRCO_CALIB_2M      (DEVINFO->HFRCOCAL0 | CMU_HFRCOCTRL_CLKDIV_DIV2)
#define HFRCO_CALIB_4M      (DEVINFO->HFRCOCAL0)
#define HFRCO_CALIB_7M      (DEVINFO->HFRCOCAL3)
#define HFRCO_CALIB_13M     (DEVINFO->HFRCOCAL6)
#define HFRCO_CALIB_16M     (DEVINFO->HFRCOCAL7)
#define HFRCO_CALIB_19M     (DEVINFO->HFRCOCAL8)
#define HFRCO_CALIB_26M     (DEVINFO->HFRCOCAL10)
#define HFRCO_CALIB_32M     (DEVINFO->HFRCOCAL11)
#define HFRCO_CALIB_38M     (DEVINFO->HFRCOCAL12)
#define HFRCO_CALIB_48M     (DEVINFO->HFRCOCAL13)
#define HFRCO_CALIB_56M     (DEVINFO->HFRCOCAL14)
#define HFRCO_CALIB_64M     (DEVINFO->HFRCOCAL15)
#define HFRCO_CALIB_72M     (DEVINFO->HFRCOCAL16)
#define AUXHFRCO_CALIB_1M   (DEVINFO->AUXHFRCOCAL0 | CMU_AUXHFRCOCTRL_CLKDIV_DIV4)
#define AUXHFRCO_CALIB_2M   (DEVINFO->AUXHFRCOCAL0 | CMU_AUXHFRCOCTRL_CLKDIV_DIV2)
#define AUXHFRCO_CALIB_4M   (DEVINFO->AUXHFRCOCAL0)
#define AUXHFRCO_CALIB_7M   (DEVINFO->AUXHFRCOCAL3)
#define AUXHFRCO_CALIB_13M  (DEVINFO->AUXHFRCOCAL6)
#define AUXHFRCO_CALIB_16M  (DEVINFO->AUXHFRCOCAL7)
#define AUXHFRCO_CALIB_19M  (DEVINFO->AUXHFRCOCAL8)
#define AUXHFRCO_CALIB_26M  (DEVINFO->AUXHFRCOCAL10)
#define AUXHFRCO_CALIB_32M  (DEVINFO->AUXHFRCOCAL11)
#define AUXHFRCO_CALIB_38M  (DEVINFO->AUXHFRCOCAL12)
#define AUXHFRCO_CALIB_48M  (DEVINFO->AUXHFRCOCAL13)
#define AUXHFRCO_CALIB_50M  (DEVINFO->AUXHFRCOCAL14)
#define USHFRCO_CALIB_16M   (DEVINFO->USHFRCOCAL7)
#define USHFRCO_CALIB_32M   (DEVINFO->USHFRCOCAL11)
#define USHFRCO_CALIB_48M   (DEVINFO->USHFRCOCAL13)
#define USHFRCO_CALIB_50M   (DEVINFO->USHFRCOCAL14)

#define LFXO_CTUNE_TO_PF(x)  (8.f + (x) * 0.25f)
#define HFXO_CTUNE_TO_PF(x)  (8.7f + (x) * 0.084f)
#define HFXO_IBTRIM_TO_UA(x) (1280 * (((x) & 0x600) >> 9) + 2 * (((x) & 0x0FF) >> 0))

void cmu_init();
void cmu_update_clocks();
void cmu_config_waitstates(uint32_t ulFrequency);

void cmu_hfrco_calib(uint32_t ulCalibration, uint32_t ulTargetFrequency);

void cmu_ushfrco_calib(uint8_t ubEnable, uint32_t ulCalibration, uint32_t ulTargetFrequency);

void cmu_auxhfrco_calib(uint8_t ubEnable, uint32_t ulCalibration, uint32_t ulTargetFrequency);

void cmu_hfxo_startup_calib(uint16_t usIBTrim, uint16_t usCTune);
float cmu_hfxo_get_startup_current();
float cmu_hfxo_get_startup_cap();
void cmu_hfxo_steady_calib(uint16_t usIBTrim, uint16_t usCTune);
float cmu_hfxo_get_steady_current();
float cmu_hfxo_get_steady_cap();
uint16_t cmu_hfxo_get_pda_ibtrim(uint8_t ubTrigger);
float cmu_hfxo_get_pda_current(uint8_t ubTrigger);
uint16_t cmu_hfxo_get_pma_ibtrim();
float cmu_hfxo_get_pma_current();

void cmu_lfxo_calib(uint8_t ubCTune);
float cmu_lfxo_get_cap();

#endif
