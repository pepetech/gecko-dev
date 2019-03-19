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
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        PN532_SELECT();
        usart0_spi_transfer_byte(0x00);
        PN532_UNSELECT();
    }
}

uint8_t pn532_writeCommand(uint8_t ubCommand, uint8_t *pubParameters, uint8_t ubNParameters)
{
    uint8_t *pubCommandBuf = (uint8_t*)malloc(1 + ubNParameters);

    if(!pubCommandBuf)
    {
        return 0;
    }


    pubCommandBuf[0] = ubCommand;
    for(uint8_t i = 0; i < ubNParameters; i++)
    {
        pubCommandBuf[1 + i] = pubParameters[i];
    }
    
    pn532_writeFrame(pubCommandBuf, 1 + ubNParameters);

    uint64_t ullTimeoutStart = g_ullSystemTick;
    while(!pn532_ready()) 
    {
        if (g_ullSystemTick > (ullTimeoutStart + PN532_ACKTIMEOUT)) 
        {
            free(pubCommandBuf);
            return PN532_TIMEOUT;
        }
    }

    if(pn532_readAck() == PN532_INVALID_ACK) 
    {
        free(pubCommandBuf);
        return  PN532_INVALID_ACK;
    }

    free(pubCommandBuf);
    return 1;
}
uint8_t pn532_readResponse(uint8_t ubCommand, uint8_t *pubBuf, uint8_t ubLength)
{
    uint64_t ullTimeoutStart = g_ullSystemTick;
    while(!pn532_ready()) 
    {
        if (g_ullSystemTick > (ullTimeoutStart + PN532_RESPONSETIMEOUT)) 
        {
            return PN532_TIMEOUT;
        }
    }

    uint8_t *pubFrameBuf = (uint8_t*)malloc(ubLength + 2);


    if(!pubFrameBuf) 
    {
        return 0;
    }

    pn532_readFrame(pubFrameBuf, ubLength + 2);

    if(pubFrameBuf[0] != PN532_PN532HOST) 
    {
        free(pubFrameBuf);
        return PN532_INVALID_FRAME;
    }
    if (pubFrameBuf[1] != (ubCommand + 1)) 
    {
        free(pubFrameBuf);
        return PN532_INVALID_FRAME;
    }
    
    for(uint8_t i = 0; i < ubLength; i++)
    {
        pubBuf[i] = pubFrameBuf[2 + i];
    }
    
    free(pubFrameBuf);
    return 1;
}

void pn532_writeFrame(uint8_t *pubPayload, uint8_t ubLength)
{
    uint8_t *pubBuf = (uint8_t*)malloc(9 + ubLength);
    
    if(!pubBuf) 
    {
        return;
    }
    

    pubBuf[0] = PN532_SPI_DATA_WRITE;        // data writing (this byte is used for spi frames only)
    pubBuf[1] = PN532_PREAMBLE;              // Preamble
    pubBuf[2] = PN532_STARTCODE1;            // Start of Packet Code
    pubBuf[3] = PN532_STARTCODE2;            // Start of Packet Code byte 2
    pubBuf[4] = ubLength + 1;                // Packet Length: TFI + DATA
    pubBuf[5] = ~pubBuf[4] + 1;               // Packet Length Checksum
    pubBuf[6] = PN532_HOSTPN532;             // TFI, Specific PN532 Frame Identifier

    uint8_t ubCheckSum = PN532_HOSTPN532;   // sum of TFI + DATA

    for(uint8_t i = 0; i < ubLength; i++)
    {
        pubBuf[7 + i] = pubPayload[i];        // Packet Data
        ubCheckSum += pubPayload[i];
    }

    ubCheckSum = ~ubCheckSum + 1;

    pubBuf[7+ubLength] = ubCheckSum;         // Packet Data Checksum
    pubBuf[8+ubLength] = PN532_POSTAMBLE;    // Postamble

    delay_ms(1);

    PN532_SELECT(); // wake up PN532               
    usart0_spi_write(pubBuf, 9 + ubLength);
    PN532_UNSELECT();

    free(pubBuf);
}
uint8_t pn532_readFrame(uint8_t *pubPayload, uint8_t ubMaxLength)
{
    uint8_t ubPreamble[4];

    ubPreamble[0] = PN532_SPI_DATA_READ;        // data writing (this byte is used for spi frames only)
    ubPreamble[1] = PN532_PREAMBLE;              // Preamble
    ubPreamble[2] = PN532_STARTCODE1;            // Start of Packet Code
    ubPreamble[3] = PN532_STARTCODE2;            // Start of Packet Code byte 2

    uint8_t ubPostamble[2];

    delay_ms(1);

    PN532_SELECT(); // Wake up PN532
    
    usart0_spi_write(ubPreamble, 4);    // write "header"
    
    uint8_t ubLength = usart0_spi_transfer_byte(0x00);  // read length byte
    
    if(((usart0_spi_transfer_byte(0x00) + ubLength) & 0xFF) != 0)    // read length byte checksum and verify ubLength
    {
        PN532_UNSELECT();

        DBGPRINTLN_CTX("INVALID FRAME");

        return PN532_INVALID_FRAME;
    }
    if(ubLength > ubMaxLength)  // verify that payload does not exceed buffer size
    {
        PN532_UNSELECT();

        DBGPRINTLN_CTX("Frame too long");

        return PN532_NO_SPACE;
    }

    usart0_spi_read(pubPayload, ubLength);   // read payload
    usart0_spi_read(ubPostamble, 2);    // read payload checksum and postamble

    PN532_UNSELECT();

    uint8_t ubChecksum = 0;

    for(uint8_t i = 0; i < ubLength; i++)   // calculate checksum
    {
        ubChecksum += pubPayload[i];    
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

    delay_ms(1);

    PN532_SELECT();
    
    usart0_spi_transfer_byte(PN532_SPI_DATA_READ);
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
        
        delay_ms(1);
        
        PN532_SELECT();
        usart0_spi_write(ubNack, 6);
        PN532_UNSELECT();
    }
    else
    {
        uint8_t ubAck[6] = {0, 0, 0xFF, 0, 0xFF, 0};
        
        delay_ms(1);

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
    delay_ms(1);

    PN532_SELECT();
    usart0_spi_transfer_byte(PN532_SPI_STATUS_READ);
    uint8_t status = usart0_spi_transfer_byte(0x00) & 1;
    PN532_UNSELECT();

    return status;
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