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
    uint8_t ubCommandBuf[1 + ubNParameters];

    ubCommandBuf[0] = ubCommand;
    for(uint8_t i = 0; i < ubNParameters; i++)
    {
        ubCommandBuf[1 + i] = ubParameters[i];
    }
    
    pn532_writeFrame(ubCommandBuf, 1 + ubNParameters);

    uint64_t ullTimeoutStart = g_ullSystemTick;
    while(!pn532_ready()) 
    {
        if (g_ullSystemTick > (g_ullSystemTick + PN532_ACKTIMEOUT)) 
        {
            return PN532_TIMEOUT;
        }
    }

    if(pn532_readAck() == PN532_INVALID_ACK) 
    {
        return  PN532_INVALID_ACK;
    }

    return 1;
}
uint8_t pn532_readResponse(uint8_t ubCommand, uint8_t* ubBuf, uint8_t ubLength)
{
    uint64_t ullTimeoutStart = g_ullSystemTick;
    while(!pn532_ready()) 
    {
        if (g_ullSystemTick > (g_ullSystemTick + PN532_RESPONSETIMEOUT)) 
        {
            return PN532_TIMEOUT;
        }
    }

    uint8_t ubFrameBuf[ubLength + 2];

    pn532_readFrame(ubFrameBuf, ubLength + 2);

    if(ubFrameBuf[0] != PN532_PN532HOST) 
    {
        return PN532_INVALID_FRAME;
    }
    if (ubFrameBuf[1] != (ubCommand + 1)) 
    {
        return PN532_INVALID_FRAME;
    }
    
    for(uint8_t i = 0; i < ubLength; i++)
    {
        ubBuf[i] = ubFrameBuf[2 + i];
    }
    
    return 1;
}

void pn532_writeFrame(uint8_t* ubPayload, uint8_t ubLength)
{
    uint8_t ubBuf[9+ubLength];

    ubBuf[0] = PN532_SPI_DATA_WRITE;        // data writing (this byte is used for spi frames only)
    ubBuf[1] = PN532_PREAMBLE;              // Preamble
    ubBuf[2] = PN532_STARTCODE1;            // Start of Packet Code
    ubBuf[3] = PN532_STARTCODE2;            // Start of Packet Code byte 2
    ubBuf[4] = ubLength + 1;                // Packet Length: TFI + DATA
    ubBuf[5] = ~ubBuf[4] + 1;               // Packet Length Checksum
    ubBuf[6] = PN532_HOSTPN532;             // TFI, Specific PN532 Frame Identifier

    uint8_t ubCheckSum = PN532_HOSTPN532;   // sum of TFI + DATA

    for(uint8_t i = 0; i < ubLength; i++)
    {
        ubBuf[7 + i] = ubPayload[i];        // Packet Data
        ubCheckSum += ubPayload[i];
    }

    ubCheckSum = ~ubCheckSum + 1;

    ubBuf[7+ubLength] = ubCheckSum;         // Packet Data Checksum
    ubBuf[8+ubLength] = PN532_POSTAMBLE;    // Postamble

    PN532_SELECT(); // wake up PN532
    delay_ms(1);               
    usart0_spi_write(ubBuf, 9 + ubLength);
    PN532_UNSELECT();
}
uint8_t pn532_readFrame(uint8_t* ubPayload, uint8_t ubMaxLength)
{
    uint8_t ubPreamble[4];

    ubPreamble[0] = PN532_SPI_DATA_READ;        // data writing (this byte is used for spi frames only)
    ubPreamble[1] = PN532_PREAMBLE;              // Preamble
    ubPreamble[2] = PN532_STARTCODE1;            // Start of Packet Code
    ubPreamble[3] = PN532_STARTCODE2;            // Start of Packet Code byte 2

    uint8_t ubPostamble[2];

    PN532_SELECT(); // Wake up PN532

    usart0_spi_write(ubPreamble, 4);    // write "header"
    
    uint8_t ubLength = usart0_spi_transfer_byte(0x00);  // read length byte
    
    if((usart0_spi_transfer_byte(0x00) + ubLength) != 0)    // read length byte checksum and verify ubLength
    {
        PN532_UNSELECT();

        return PN532_INVALID_FRAME;
    }
    if(ubLength > ubMaxLength)  // verify that payload does not exceed buffer size
    {
        PN532_UNSELECT();

        return PN532_NO_SPACE;
    }

    usart0_spi_read(ubPayload, ubLength);   // read payload
    usart0_spi_read(ubPostamble, 2);    // read payload checksum and postamble

    PN532_UNSELECT();

    uint8_t ubChecksum = 0;

    for(uint8_t i = 0; i < ubLength; i++)   // calculate checksum
    {
        ubChecksum += ubPayload[i];    
    }
    ubChecksum = ~ubChecksum + 1;

    if (ubPostamble[0] != ubChecksum)   // verify checksum \ data integrity
    {
        return PN532_INVALID_FRAME;
    }
    
    return ubLength;
}

uint8_t pn532_readAck()
{
    uint8_t ubAck[6] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ubAckBuf[6] = {0, 0, 0, 0, 0, 0};

    PN532_SELECT();
    usart0_spi_read(ubAckBuf, 6);
    PN532_UNSELECT();

    for(uint8_t i = 0; i < 6; i++) 
    {
        if(ubAckBuf[i] != ubAck[i])
        {
            return PN532_INVALID_ACK;
        }
    }

    return 1;
}
void pn532_writeAck(uint8_t ubNack)
{
    if(ubNack)
    {
        uint8_t ubNack[6] = {0, 0, 0xFF, 0, 0xFF, 0};
        PN532_SELECT();
        usart0_spi_write(ubNack, 6);
        PN532_UNSELECT();
    }
    else
    {
        uint8_t ubAck[6] = {0, 0, 0xFF, 0, 0xFF, 0};
        PN532_SELECT();
        usart0_spi_write(ubAck, 6);
        PN532_UNSELECT();
    }
}

uint8_t pn532_ready()
{
#ifdef  PN532_READY_HW
    return PN532_READY();
#else   // PN532_READY_HW
    usart0_spi_transfer_byte(PN532_SPI_STATUS_READ);
    return usart0_spi_transfer_byte(0x00) & 1;
#endif  // PN532_READY_HW
}

// pn532 funcions
uint32_t pn532_getVersion()
{
    uint8_t ubBuf[4];

    if(pn532_writeCommand(PN532_COMMAND_GETFIRMWAREVERSION, NULL, 0) != 1)
    {
        return 0;
    }
    if(pn532_readResponse(PN532_COMMAND_GETFIRMWAREVERSION, ubBuf, 4) != 1)
    {
        return 0;
    }

    return (uint32_t)(ubBuf[0] << 24) | (uint32_t)(ubBuf[1] << 16) | (uint32_t)(ubBuf[2] << 8) | (uint32_t)ubBuf[3];
}