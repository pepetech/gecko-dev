#include "usart.h"

#if defined(USART0_MODE_SPI)
void usart0_init(uint32_t ulBaud, uint8_t ubMode, uint8_t ubBitMode, int8_t bMISOLocation, int8_t bMOSILocation, uint8_t ubCLKLocation)
{
    if(bMISOLocation > AFCHANLOC_MAX)
        return;

    if(bMOSILocation > AFCHANLOC_MAX)
        return;

    if(ubCLKLocation > AFCHANLOC_MAX)
        return;

    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_USART0;

    USART0->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX | UART_CMD_TXTRIDIS | USART_CMD_RXBLOCKDIS | USART_CMD_TXDIS | USART_CMD_RXDIS;

    USART0->CTRL = USART_CTRL_TXBIL_EMPTY | USART_CTRL_CSMA_NOACTION | (ubBitMode == USART_SPI_MSB_FIRST ? USART_CTRL_MSBF : 0) | (ubMode & 1 ? USART_CTRL_CLKPHA_SAMPLETRAILING : USART_CTRL_CLKPHA_SAMPLELEADING) | (ubMode & 2 ? USART_CTRL_CLKPOL_IDLEHIGH : USART_CTRL_CLKPOL_IDLELOW) | USART_CTRL_SYNC;
    USART0->FRAME = USART_FRAME_DATABITS_EIGHT;
    USART0->CLKDIV = ((HFPER_CLOCK_FREQ / (2 * ulBaud)) - 1) << 8;

    USART0->ROUTEPEN = (bMISOLocation >= 0 ? USART_ROUTEPEN_RXPEN : 0) | (bMOSILocation >= 0 ? USART_ROUTEPEN_TXPEN : 0) | USART_ROUTEPEN_CLKPEN;
    USART0->ROUTELOC0 = ((uint32_t)(bMISOLocation >= 0 ? bMISOLocation : 0) << _USART_ROUTELOC0_RXLOC_SHIFT) | ((uint32_t)(bMOSILocation >= 0 ? bMOSILocation : 0) << _USART_ROUTELOC0_TXLOC_SHIFT) | ((uint32_t)ubCLKLocation << _USART_ROUTELOC0_CLKLOC_SHIFT);

    USART0->CMD = USART_CMD_MASTEREN | USART_CMD_TXEN | USART_CMD_RXEN;
}
uint8_t usart0_spi_transfer_byte(const uint8_t ubData)
{
    while(!(USART0->STATUS & USART_STATUS_TXBL));

    USART0->TXDATA = ubData;

    while(!(USART0->STATUS & USART_STATUS_TXC));

    return USART0->RXDATA;
}
void usart0_spi_transfer(const uint8_t* pubSrc, uint32_t ulSize, uint8_t* pubDst)
{
	if(pubSrc)
	{
		while(ulSize--)
		{
			if(pubDst)
				*(pubDst++) = usart0_spi_transfer_byte(*(pubSrc++));
			else
				usart0_spi_transfer_byte(*(pubSrc++));
		}
	}
	else if(pubDst)
	{
		while(ulSize--)
			*(pubDst++) = usart0_spi_transfer_byte(0x00);
	}
}
#else   // USART0_MODE
static volatile uint8_t *pubUSART0DMABuffer = NULL;
static volatile uint8_t *pubUSART0FIFO = NULL;
static volatile uint16_t usUSART0FIFOWritePos, usUART2FIFOReadPos;

void usart0_init(uint32_t ulBaud, uint32_t ulFrameSettings, int8_t bRXLocation, int8_t bTXLocation, int8_t bCTSLocation, int8_t bRTSLocation)
{
    if(bRXLocation > AFCHANLOC_MAX)
        return;

    if(bTXLocation > AFCHANLOC_MAX)
        return;

    if(bCTSLocation > AFCHANLOC_MAX)
        return;

    if(bRTSLocation > AFCHANLOC_MAX)
        return;

    CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_USART0;

    USART0->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX | UART_CMD_TXTRIDIS | USART_CMD_RXBLOCKDIS | USART_CMD_TXDIS | USART_CMD_RXDIS;

    free((uint8_t *)pubUSART0DMABuffer);
    free((uint8_t *)pubUSART0FIFO);

    pubUSART0DMABuffer = (volatile uint8_t *)malloc(USART0_DMA_RX_BUFFER_SIZE);

    if(!pubUSART0DMABuffer)
        return;

    memset((uint8_t *)pubUSART0DMABuffer, 0, USART0_DMA_RX_BUFFER_SIZE);

    pubUSART0FIFO = (volatile uint8_t *)malloc(USART0_FIFO_SIZE);

    if(!pubUSART0FIFO)
    {
        free((void *)pubUSART0DMABuffer);

        return;
    }

    memset((uint8_t *)pubUSART0FIFO, 0, USART0_FIFO_SIZE);

    usUSART0FIFOWritePos = 0;
    usUART2FIFOReadPos = 0;

    USART0->CTRL = USART_CTRL_TXBIL_EMPTY | USART_CTRL_CSMA_NOACTION | UART_CTRL_OVS_X16;
    USART0->CTRLX = (bCTSLocation >= 0 ? USART_CTRLX_CTSEN : 0);
    USART0->FRAME = ulFrameSettings;
    USART0->CLKDIV = ((HFPER_CLOCK_FREQ / (16 * ulBaud)) - 1) << 8;

    USART0->TIMECMP0 = UART_TIMECMP0_TSTOP_RXACT | UART_TIMECMP0_TSTART_RXEOF | 0x08; // RX Timeout after 8 baud times

    USART0->ROUTEPEN = (bRXLocation >= 0 ? USART_ROUTEPEN_RXPEN : 0) | (bTXLocation >= 0 ? USART_ROUTEPEN_TXPEN : 0) | (bCTSLocation >= 0 ? USART_ROUTEPEN_CTSPEN : 0) | (bRTSLocation >= 0 ? USART_ROUTEPEN_RTSPEN : 0);
    USART0->ROUTELOC0 = ((uint32_t)(bRXLocation >= 0 ? bRXLocation : 0) << _USART_ROUTELOC0_RXLOC_SHIFT) | ((uint32_t)(bTXLocation >= 0 ? bTXLocation : 0) << _USART_ROUTELOC0_TXLOC_SHIFT);
    USART0->ROUTELOC1 = ((uint32_t)(bCTSLocation >= 0 ? bCTSLocation : 0) << _USART_ROUTELOC1_CTSLOC_SHIFT) | ((uint32_t)(bRTSLocation >= 0 ? bRTSLocation : 0) << _USART_ROUTELOC1_RTSLOC_SHIFT);

    USART0->CMD = (bTXLocation >= 0 ? USART_CMD_TXEN : 0) | (bRXLocation >= 0 ? USART_CMD_RXEN : 0);
}
void usart0_write_byte(const uint8_t ubData)
{
    while(!(USART0->STATUS & USART_STATUS_TXBL));

    USART0->TXDATA = ubData;
}
uint8_t usart0_read_byte()
{
    if(!usart0_available())
        return 0;

    uint8_t ubData;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        ubData = pubUSART0FIFO[usUART2FIFOReadPos++];

        if(usUART2FIFOReadPos >= USART0_FIFO_SIZE)
            usUART2FIFOReadPos = 0;
    }

    return ubData;
}
uint32_t usart0_available()
{
    return (USART0_FIFO_SIZE + usUSART0FIFOWritePos - usUART2FIFOReadPos) % USART0_FIFO_SIZE;
}
void usart0_flush()
{
    usUART2FIFOReadPos = usUSART0FIFOWritePos = 0;
}
#endif  // USART0_MODE