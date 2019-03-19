#ifndef __PN532_H__
#define __PN532_H__

#include <em_device.h>
#include "usart.h"
#include "systick.h"
#include "debug_macros.h"

#define PN532_DEBUG

// PN532 Commands
// Miscellaneous
#define PN532_COMMAND_DIAGNOSE              0x00
#define PN532_COMMAND_GETFIRMWAREVERSION    0x02
#define PN532_COMMAND_GETGENERALSTATUS      0x04  
#define PN532_COMMAND_READREGISTER          0x06
#define PN532_COMMAND_WRITEREGISTER         0x08
#define PN532_COMMAND_READGPIO              0x0C
#define PN532_COMMAND_WRITEGPIO             0x0E
#define PN532_COMMAND_SETSERIALBAUDRATE     0x10
#define PN532_COMMAND_SETPARAMETERS         0x12
#define PN532_COMMAND_SAMCONFIGURATION      0x14
#define PN532_COMMAND_POWERDOWN             0x16
// RF communication
#define PN532_COMMAND_RFCONFIGURATION       0x32
#define PN532_COMMAND_RFREGULATIONTEST      0x58
// Initiator
#define PN532_COMMAND_INJUMPFORDEP          0x56
#define PN532_COMMAND_INJUMPFORPSL          0x46
#define PN532_COMMAND_INLISTPASSIVETARGET   0x4A
//#define PN532_RESPONSE_INLISTPASSIVETARGET  0x4B
#define PN532_COMMAND_INATR                 0x50
#define PN532_COMMAND_INPSL                 0x4E
#define PN532_COMMAND_INDATAEXCHANGE        0x40
//#define PN532_RESPONSE_INDATAEXCHANGE       0x41
#define PN532_COMMAND_INCOMMUNICATETHRU     0x42
#define PN532_COMMAND_INDESELECT            0x44
#define PN532_COMMAND_INRELEASE             0x52
#define PN532_COMMAND_INSELECT              0x54
#define PN532_COMMAND_INAUTOPOLL            0x60
// Target
#define PN532_COMMAND_TGINITASTARGET        0x8C
#define PN532_COMMAND_TGSETGENERALBYTES     0x92
#define PN532_COMMAND_TGGETDATA             0x86
#define PN532_COMMAND_TGSETDATA             0x8E
#define PN532_COMMAND_TGSETMETADATA         0x94
#define PN532_COMMAND_TGGETINITIATORCOMMAND 0x88
#define PN532_COMMAND_TGRESPONSETOINITIATOR 0x90
#define PN532_COMMAND_TGGETTARGETSTATUS     0x8A

// Error code list
// Time Out, the target has not answered 0x01 
// A CRC error has been detected by the CIU 0x02 
// A Parity error has been detected by the CIU 0x03 
// During an anti-collision/select operation (ISO/IEC14443-3 Type A and ISO/IEC18092 106 kbps passive mode), an erroneous Bit Count has been detected 0x04 
// Framing error during Mifare operation 0x05 
// An abnormal bit-collision has been detected during bit wise anti-collision at 106 kbps 0x06
// Communication buffer size insufficient 0x07 
// RF Buffer overflow has been detected by the CIU (bit BufferOvfl of the register CIU_Error) 0x09 
// In active communication mode, the RF field has not been switched on in time by the counterpart (as defined in NFCIP-1 standard) 0x0A 
// RF Protocol error (cf. Error! Reference source not found.,description of the CIU_Error register) 0x0B 
// Temperature error: the internal temperature sensor has
// detected overheating, and therefore has automatically switched off the antenna drivers 0x0D
// Internal buffer overflow 0x0E 
// Invalid parameter (range, format, …) 0x10 
// DEP Protocol: The PN532 configured in target mode does not support the command received from the initiator (the command received is not one of the following: ATR_REQ, WUP_REQ, PSL_REQ, DEP_REQ, DSL_REQ, RLS_REQ Error! Reference source not found.). 0x12 
// DEP Protocol, Mifare or ISO/IEC14443-4: The data format does not match to the specification. Depending on the RF protocol used, it can be: Bad length of RF received frame; Incorrect value of PCB or PFB; Invalid or unexpected RF received frame; NAD or DID incoherence. 0x13
// Mifare: Authentication error 0x14
// ISO/IEC14443-3: UID Check byte is wrong 0x23 
// DEP Protocol: Invalid device state, the system is in a state which does not allow the operation 0x25 
// Operation not allowed in this configuration (host controller interface) 0x26
// This command is not acceptable due to the current context of the PN532 (Initiator vs. Target, unknown target number, Target not in the good state, …) 0x27
// The PN532 configured as target has been released by its initiator 0x29 
// PN532 and ISO/IEC14443-3B only: the ID of the card does not match, meaning that the expected card has been exchanged with another one. 0x2A
// PN532 and ISO/IEC14443-3B only: the card previously activated has disappeared. 0x2B
// Mismatch between the NFCID3 initiator and the NFCID3 target in DEP 212/424 kbps passive. 0x2C
// An over-current event has been detected 0x2D 
// NAD missing in DEP frame 0x2E

// pn532 interface funcions
uint8_t pn532_init();

void pn532_wakeUp();

uint8_t pn532_writeCommand(uint8_t ubCommand, uint8_t* ubParameters, uint8_t ubNParameters);
uint8_t pn532_readResponse(uint8_t ubCommand, uint8_t* ubBuf, uint8_t ubLength);

uint8_t pn532_writeFrame(uint8_t* ubPayload, uint8_t ubLength);
uint8_t pn532_readFrame(uint8_t* ubPayload, uint8_t ubLength);

uint8_t pn532_readAck();
uint8_t pn532_writeAck(uint8_t ubNack);

uint8_t pn532_ready();

// pn532 funcions
uint32_t pn532_getVersion();

#endif  // __PN532_H__