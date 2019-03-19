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
    delay(1);               
    usart0_spi_write(ubBuf, 9 + ubLength);
    PN532_UNSELECT();
}
uint8_t pn532_readFrame(uint8_t* ubPayload, uint8_t ubLength)
{
    uint64_t ullTimeoutStart = g_ullSystemTick;
    while(!pn532_ready()) 
    {
        if (g_ullSystemTick > (g_ullSystemTick + )) {
            /* code */
        }
        
    }

    digitalWrite(_ss, LOW);
    delay(1);

    int16_t result;
    do {
        write(DATA_READ);

        if (0x00 != read()      ||       // PREAMBLE
                0x00 != read()  ||       // STARTCODE1
                0xFF != read()           // STARTCODE2
           ) {

            result = PN532_INVALID_FRAME;
            break;
        }

        uint8_t length = read();
        if (0 != (uint8_t)(length + read())) {   // checksum of length
            result = PN532_INVALID_FRAME;
            break;
        }

        uint8_t cmd = command + 1;               // response command
        if (PN532_PN532TOHOST != read() || (cmd) != read()) {
            result = PN532_INVALID_FRAME;
            break;
        }

        DMSG("read:  ");
        DMSG_HEX(cmd);

        length -= 2;
        if (length > len) {
            for (uint8_t i = 0; i < length; i++) {
                DMSG_HEX(read());                 // dump message
            }
            DMSG("\nNot enough space\n");
            read();
            read();
            result = PN532_NO_SPACE;  // not enough space
            break;
        }

        uint8_t sum = PN532_PN532TOHOST + cmd;
        for (uint8_t i = 0; i < length; i++) {
            buf[i] = read();
            sum += buf[i];

            DMSG_HEX(buf[i]);
        }
        DMSG('\n');

        uint8_t checksum = read();
        if (0 != (uint8_t)(sum + checksum)) {
            DMSG("checksum is not ok\n");
            result = PN532_INVALID_FRAME;
            break;
        }
        read();         // POSTAMBLE

        result = length;
    } while (0);

    digitalWrite(_ss, HIGH);

    return result;
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
            return 0;
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

}