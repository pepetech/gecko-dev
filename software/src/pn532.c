#include "pn532.h"

#define PN532_SPI_STATUS_READ     2
#define PN532_SPI_DATA_WRITE      1
#define PN532_SPI_DATA_READ       3

// pn532 interface funcions
uint8_t pn532_init()
{
    PN532_RESET();
    delay_ms(1);
    PN532_UNRESET();

    return 1;
}

void pn532_wakeUp()
{
    PN532_SELECT();
    delay_ms(1);
    PN532_UNSELECT();
}

uint8_t pn532_writeCommand(uint8_t ubCommand, uint8_t* ubParameters, uint8_t ubNParameters)
{

}
uint8_t pn532_readResponse(uint8_t ubCommand, uint8_t* ubBuf, uint8_t ubLength)
{

}

uint8_t pn532_writeFrame(uint8_t* ubPayload, uint8_t ubLength)
{

}
uint8_t pn532_readFrame(uint8_t* ubPayload, uint8_t ubLength)
{

}

uint8_t pn532_readAck()
{

}
uint8_t pn532_writeAck(uint8_t ubNack)
{

}

uint8_t pn532_ready()
{

}

// pn532 funcions
uint32_t pn532_getVersion()
{

}