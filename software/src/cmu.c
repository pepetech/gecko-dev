#include "cmu.h"

uint32_t HFRCO_VALUE = 19000000UL;
uint32_t AUXHFRCO_VALUE = 19000000UL;
uint32_t LFRCO_VALUE = 32768UL;
uint32_t ULFRCO_VALUE = 1000UL;

uint32_t HFSRC_CLOCK_FREQ;
uint32_t HF_CLOCK_FREQ;
uint32_t HFCORE_CLOCK_FREQ;
uint32_t HFEXP_CLOCK_FREQ;
uint32_t HFPER_CLOCK_FREQ;
uint32_t HFLE_CLOCK_FREQ;
uint32_t ADC0_CLOCK_FREQ;
uint32_t DBG_CLOCK_FREQ;
uint32_t AUX_CLOCK_FREQ;
uint32_t LFA_CLOCK_FREQ;
uint32_t LETIMER0_CLOCK_FREQ;
uint32_t LFB_CLOCK_FREQ;
uint32_t LEUART0_CLOCK_FREQ;
uint32_t LFE_CLOCK_FREQ;
uint32_t RTCC_CLOCK_FREQ;

void cmu_init()
{
    // Disable HFXO if enabled
    if(CMU->STATUS & CMU_STATUS_HFXOENS)
    {
        CMU->OSCENCMD = CMU_OSCENCMD_HFXODIS;
        while(CMU->STATUS & CMU_STATUS_HFXOENS);
    }

    // Setup HFXO
    CMU->HFXOCTRL = CMU_HFXOCTRL_LOWPOWER | CMU_HFXOCTRL_PEAKDETSHUNTOPTMODE_AUTOCMD | CMU_HFXOCTRL_MODE_XTAL;
    CMU->HFXOCTRL1 = CMU_HFXOCTRL1_XTIBIASEN_DEFAULT | CMU_HFXOCTRL1_REGLVL_DEFAULT | CMU_HFXOCTRL1_PEAKDETTHR_DEFAULT;
    CMU->HFXOTIMEOUTCTRL = (7 << _CMU_HFXOTIMEOUTCTRL_PEAKDETTIMEOUT_SHIFT) | (8 << _CMU_HFXOTIMEOUTCTRL_STEADYTIMEOUT_SHIFT) | (12 << _CMU_HFXOTIMEOUTCTRL_STARTUPTIMEOUT_SHIFT);

    // Enable HFXO and wait for it to be ready
    //CMU->OSCENCMD = CMU_OSCENCMD_HFXOEN;
    //while(!(CMU->STATUS & CMU_STATUS_HFXORDY));

    // Config peripherals for the new frequency
    cmu_config_waitstates(32000000);
    msc_config_waitstates(32000000);

    // Set prescalers
    CMU->HFPRESC = CMU_HFPRESC_HFCLKLEPRESC_DIV4 | CMU_HFPRESC_PRESC_NODIVISION;
    CMU->HFCOREPRESC = 0 << _CMU_HFCOREPRESC_PRESC_SHIFT;
    CMU->HFPERPRESC = 0 << _CMU_HFPERPRESC_PRESC_SHIFT;
    CMU->HFEXPPRESC = 0 << _CMU_HFEXPPRESC_PRESC_SHIFT;

    // Switch main clock to HFXO and wait for it to be selected
    //CMU->HFCLKSEL = CMU_HFCLKSEL_HF_HFXO;
    //while((CMU->HFCLKSTATUS & _CMU_HFCLKSTATUS_SELECTED_MASK) != CMU_HFCLKSTATUS_SELECTED_HFXO);

    // Calibrate HFRCO for 32MHz
    cmu_hfrco_calib(HFRCO_CALIB_32M, 32000000);

    // Enable clock to peripherals
    CMU->CTRL |= CMU_CTRL_HFPERCLKEN;

    // Switch main clock to HFRCO and wait for it to be selected
    CMU->HFCLKSEL = CMU_HFCLKSEL_HF_HFRCO;
    while((CMU->HFCLKSTATUS & _CMU_HFCLKSTATUS_SELECTED_MASK) != CMU_HFCLKSTATUS_SELECTED_HFRCO);

    // LFE Clock
    CMU->LFECLKSEL = CMU_LFECLKSEL_LFE_ULFRCO;
}
void cmu_update_clocks()
{
    AUX_CLOCK_FREQ = AUXHFRCO_VALUE;

    switch(CMU->HFCLKSTATUS & _CMU_HFCLKSTATUS_SELECTED_MASK)
    {
        case CMU_HFCLKSTATUS_SELECTED_HFRCO:
            HFSRC_CLOCK_FREQ = HFRCO_VALUE;
        break;
        case CMU_HFCLKSTATUS_SELECTED_HFXO:
            HFSRC_CLOCK_FREQ = HFXO_VALUE;
        break;
        case CMU_HFCLKSTATUS_SELECTED_LFRCO:
            HFSRC_CLOCK_FREQ = LFRCO_VALUE;
        break;
        case CMU_HFCLKSTATUS_SELECTED_LFXO:
            HFSRC_CLOCK_FREQ = LFXO_VALUE;
        break;
    }

    HF_CLOCK_FREQ = HFSRC_CLOCK_FREQ / (((CMU->HFPRESC & _CMU_HFPRESC_PRESC_MASK) >> _CMU_HFPRESC_PRESC_SHIFT) + 1);

    switch(CMU->HFPRESC & _CMU_HFPRESC_HFCLKLEPRESC_MASK)
    {
        case CMU_HFPRESC_HFCLKLEPRESC_DIV2:
            HFLE_CLOCK_FREQ = HF_CLOCK_FREQ >> 1;
        break;
        case CMU_HFPRESC_HFCLKLEPRESC_DIV4:
            HFLE_CLOCK_FREQ = HF_CLOCK_FREQ >> 2;
        break;
    }

    HFCORE_CLOCK_FREQ = HF_CLOCK_FREQ / (((CMU->HFCOREPRESC & _CMU_HFCOREPRESC_PRESC_MASK) >> _CMU_HFCOREPRESC_PRESC_SHIFT) + 1);
    HFEXP_CLOCK_FREQ = HF_CLOCK_FREQ / (((CMU->HFEXPPRESC & _CMU_HFEXPPRESC_PRESC_MASK) >> _CMU_HFEXPPRESC_PRESC_SHIFT) + 1);
    HFPER_CLOCK_FREQ = HF_CLOCK_FREQ / (((CMU->HFPERPRESC & _CMU_HFPERPRESC_PRESC_MASK) >> _CMU_HFPERPRESC_PRESC_SHIFT) + 1);

    switch(CMU->DBGCLKSEL & _CMU_DBGCLKSEL_DBG_MASK)
    {
        case CMU_DBGCLKSEL_DBG_AUXHFRCO:
            DBG_CLOCK_FREQ = AUX_CLOCK_FREQ;
        break;
        case CMU_DBGCLKSEL_DBG_HFCLK:
            DBG_CLOCK_FREQ = HF_CLOCK_FREQ;
        break;
    }

    switch(CMU->ADCCTRL & _CMU_ADCCTRL_ADC0CLKSEL_MASK)
    {
        case CMU_ADCCTRL_ADC0CLKSEL_DISABLED:
            ADC0_CLOCK_FREQ = 0;
        break;
        case CMU_ADCCTRL_ADC0CLKSEL_AUXHFRCO:
            ADC0_CLOCK_FREQ = AUX_CLOCK_FREQ;
        break;
        case CMU_ADCCTRL_ADC0CLKSEL_HFXO:
            ADC0_CLOCK_FREQ = HFXO_VALUE;
        break;
        case CMU_ADCCTRL_ADC0CLKSEL_HFSRCCLK:
            ADC0_CLOCK_FREQ = HFSRC_CLOCK_FREQ;
        break;
    }

    switch(CMU->LFACLKSEL & _CMU_LFACLKSEL_LFA_MASK)
    {
        case CMU_LFACLKSEL_LFA_DISABLED:
            LFA_CLOCK_FREQ = 0;
        break;
        case CMU_LFACLKSEL_LFA_LFRCO:
            LFA_CLOCK_FREQ = LFRCO_VALUE;
        break;
        case CMU_LFACLKSEL_LFA_LFXO:
            LFA_CLOCK_FREQ = LFXO_VALUE;
        break;
        case CMU_LFACLKSEL_LFA_ULFRCO:
            LFA_CLOCK_FREQ = ULFRCO_VALUE;
        break;
    }

    LETIMER0_CLOCK_FREQ = LFA_CLOCK_FREQ << ((CMU->LFAPRESC0 & _CMU_LFAPRESC0_LETIMER0_MASK) >> _CMU_LFAPRESC0_LETIMER0_SHIFT);

    switch(CMU->LFBCLKSEL & _CMU_LFBCLKSEL_LFB_MASK)
    {
        case CMU_LFBCLKSEL_LFB_DISABLED:
            LFB_CLOCK_FREQ = 0;
        break;
        case CMU_LFBCLKSEL_LFB_LFRCO:
            LFB_CLOCK_FREQ = LFRCO_VALUE;
        break;
        case CMU_LFBCLKSEL_LFB_LFXO:
            LFB_CLOCK_FREQ = LFXO_VALUE;
        break;
        case CMU_LFBCLKSEL_LFB_HFCLKLE:
            LFB_CLOCK_FREQ = HFLE_CLOCK_FREQ;
        break;
        case CMU_LFBCLKSEL_LFB_ULFRCO:
            LFB_CLOCK_FREQ = ULFRCO_VALUE;
        break;
    }

    LEUART0_CLOCK_FREQ = LFB_CLOCK_FREQ << ((CMU->LFBPRESC0 & _CMU_LFBPRESC0_LEUART0_MASK) >> _CMU_LFBPRESC0_LEUART0_SHIFT);

    switch(CMU->LFECLKSEL & _CMU_LFECLKSEL_LFE_MASK)
    {
        case CMU_LFECLKSEL_LFE_DISABLED:
            LFE_CLOCK_FREQ = 0;
        break;
        case CMU_LFECLKSEL_LFE_LFRCO:
            LFE_CLOCK_FREQ = LFRCO_VALUE;
        break;
        case CMU_LFECLKSEL_LFE_LFXO:
            LFE_CLOCK_FREQ = LFXO_VALUE;
        break;
        case CMU_LFECLKSEL_LFE_ULFRCO:
            LFE_CLOCK_FREQ = ULFRCO_VALUE;
        break;
    }

    RTCC_CLOCK_FREQ = LFE_CLOCK_FREQ << ((CMU->LFEPRESC0 & _CMU_LFEPRESC0_RTCC_MASK) >> _CMU_LFEPRESC0_RTCC_SHIFT);
}
void cmu_config_waitstates(uint32_t ulFrequency)
{
    if(ulFrequency <= 32000000)
        CMU->CTRL &= ~CMU_CTRL_WSHFLE;
    else
        CMU->CTRL |= CMU_CTRL_WSHFLE;
}

void cmu_hfrco_calib(uint32_t ulCalibration, uint32_t ulTargetFrequency)
{
    while(CMU->SYNCBUSY & CMU_SYNCBUSY_HFRCOBSY);

    CMU->HFRCOCTRL = ulCalibration;

    while(CMU->SYNCBUSY & CMU_SYNCBUSY_HFRCOBSY);

    HFRCO_VALUE = ulTargetFrequency;
}

void cmu_auxhfrco_calib(uint8_t ubEnable, uint32_t ulCalibration, uint32_t ulTargetFrequency)
{
    if(!ubEnable)
    {
        CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCODIS;
        while(CMU->STATUS & CMU_STATUS_AUXHFRCOENS);

        return;
    }

    while(CMU->SYNCBUSY & CMU_SYNCBUSY_AUXHFRCOBSY);

    CMU->AUXHFRCOCTRL = ulCalibration;

    while(CMU->SYNCBUSY & CMU_SYNCBUSY_AUXHFRCOBSY);

    if(ubEnable && !(CMU->STATUS & CMU_STATUS_AUXHFRCOENS))
    {
        CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;

        while(!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));
    }

    AUXHFRCO_VALUE = ulTargetFrequency;
}

void cmu_hfxo_startup_calib(uint16_t usIBTrim, uint16_t usCTune)
{
    if(CMU->STATUS & CMU_STATUS_HFXOENS)
        return;

    CMU->HFXOSTARTUPCTRL = (CMU->HFXOSTARTUPCTRL & ~(_CMU_HFXOSTARTUPCTRL_CTUNE_MASK | _CMU_HFXOSTARTUPCTRL_IBTRIMXOCORE_MASK)) | (((uint32_t)usCTune << _CMU_HFXOSTARTUPCTRL_CTUNE_SHIFT) & _CMU_HFXOSTARTUPCTRL_CTUNE_MASK) | (((uint32_t)usIBTrim << _CMU_HFXOSTARTUPCTRL_IBTRIMXOCORE_SHIFT) & _CMU_HFXOSTARTUPCTRL_IBTRIMXOCORE_MASK);
}
float cmu_hfxo_get_startup_current()
{
    return HFXO_IBTRIM_TO_UA((CMU->HFXOSTARTUPCTRL & _CMU_HFXOSTARTUPCTRL_IBTRIMXOCORE_MASK) >> _CMU_HFXOSTARTUPCTRL_IBTRIMXOCORE_SHIFT);
}
float cmu_hfxo_get_startup_cap()
{
    return HFXO_CTUNE_TO_PF((CMU->HFXOSTARTUPCTRL & _CMU_HFXOSTARTUPCTRL_CTUNE_MASK) >> _CMU_HFXOSTARTUPCTRL_CTUNE_SHIFT);
}
void cmu_hfxo_steady_calib(uint16_t usIBTrim, uint16_t usCTune)
{
    if(CMU->STATUS & CMU_STATUS_HFXOENS)
        return;

    CMU->HFXOSTEADYSTATECTRL = (CMU->HFXOSTEADYSTATECTRL & ~(_CMU_HFXOSTEADYSTATECTRL_CTUNE_MASK | _CMU_HFXOSTEADYSTATECTRL_IBTRIMXOCORE_MASK)) | (((uint32_t)usCTune << _CMU_HFXOSTEADYSTATECTRL_CTUNE_SHIFT) & _CMU_HFXOSTEADYSTATECTRL_CTUNE_MASK) | (((uint32_t)usIBTrim << _CMU_HFXOSTEADYSTATECTRL_IBTRIMXOCORE_SHIFT) & _CMU_HFXOSTEADYSTATECTRL_IBTRIMXOCORE_MASK);
}
float cmu_hfxo_get_steady_current()
{
    return HFXO_IBTRIM_TO_UA((CMU->HFXOSTEADYSTATECTRL & _CMU_HFXOSTEADYSTATECTRL_IBTRIMXOCORE_MASK) >> _CMU_HFXOSTEADYSTATECTRL_IBTRIMXOCORE_SHIFT);
}
float cmu_hfxo_get_steady_cap()
{
    return HFXO_CTUNE_TO_PF((CMU->HFXOSTEADYSTATECTRL & _CMU_HFXOSTEADYSTATECTRL_CTUNE_MASK) >> _CMU_HFXOSTEADYSTATECTRL_CTUNE_SHIFT);
}
uint16_t cmu_hfxo_get_pda_ibtrim(uint8_t ubTrigger)
{
    if(!(CMU->STATUS & CMU_STATUS_HFXOENS))
        return 0;

    if(ubTrigger)
    {
        CMU->CMD = CMU_CMD_HFXOPEAKDETSTART;
        while(!(CMU->STATUS & CMU_STATUS_HFXOPEAKDETRDY));
    }

    return (CMU->HFXOTRIMSTATUS & _CMU_HFXOTRIMSTATUS_IBTRIMXOCORE_MASK) >> _CMU_HFXOTRIMSTATUS_IBTRIMXOCORE_SHIFT;
}
float cmu_hfxo_get_pda_current(uint8_t ubTrigger)
{
    if(!(CMU->STATUS & CMU_STATUS_HFXOENS))
        return 0;

    return HFXO_IBTRIM_TO_UA(cmu_hfxo_get_pda_ibtrim(ubTrigger));
}

void cmu_lfxo_calib(uint8_t ubCTune)
{
    if(CMU->STATUS & CMU_STATUS_LFXOENS)
        return;

    float fCLoad = LFXO_CTUNE_TO_PF(ubCTune) / 2.f;
    uint8_t ubGain = 0;

    if(fCLoad <= 6.f)
        ubGain = 0;
    else if(fCLoad <= 8.f)
        ubGain = 1;
    else if(fCLoad <= 12.5f)
        ubGain = 2;
    else if(fCLoad <= 18.f)
        ubGain = 3;
    else
        return;

    CMU->LFXOCTRL = (CMU->LFXOCTRL & ~(_CMU_LFXOCTRL_GAIN_MASK | _CMU_LFXOCTRL_TUNING_MASK)) | ((uint32_t)ubGain << _CMU_LFXOCTRL_GAIN_SHIFT) | (((uint32_t)ubCTune << _CMU_LFXOCTRL_TUNING_SHIFT) & _CMU_LFXOCTRL_TUNING_MASK);
}
float cmu_lfxo_get_cap()
{
    return LFXO_CTUNE_TO_PF((CMU->LFXOCTRL & _CMU_LFXOCTRL_TUNING_MASK) >> _CMU_LFXOCTRL_TUNING_SHIFT);
}