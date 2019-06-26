/***************************************************************************//**
 * @file
 * @brief   Contains call-back functions from the EFM32 USB Library.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <stddef.h>
#include "descriptors.h"
#include "usb_user.h"

// -----------------------------------------------------------------------------
// Variables

/// Byte holding the current USB_API interrupts
uint32_t USBXCORE_apiIntValue;

/// Enable or disable status of USB_API interrupts
uint32_t USBXCORE_apiEa;

/// Buffer to hold overflow rx data if a ZLP read returns more than zero bytes.
UBUF(USBXCORE_overflowBuffer, USB_FS_BULK_EP_MAXSIZE);

/// Boolean indicating whether a ZLP read is active.
bool USBXCORE_zlpActive = false;

/// Boolean indicating if data was received while expecting a ZLP.
bool USBXCORE_rxOverflowPacketAvailable = false;

/// Size of Rx Overflow Packet
uint32_t USBXCORE_rxOverflowPacketSize = 0;

/// Pointer to variable holding number of bytes written
uint32_t* USBXCORE_byteCountInPtr;

/// Pointer to variable holding number of bytes read
uint32_t* USBXCORE_byteCountOutPtr;

/// Number of bytes sent to USBX_blockRead() as numBytes
uint32_t USBXCORE_readSize;

/// Number of bytes sent to USBX_blockWrite() as numBytes
uint32_t USBXCORE_writeSize;

// -----------------------------------------------------------------------------
// Extern Variable Declarations

extern uint8_t USBXCORE_overflowBuffer[];
extern bool USBXCORE_zlpActive;
extern bool USBXCORE_rxOverflowPacketAvailable;
extern uint32_t USBXCORE_rxOverflowPacketSize;

// -----------------------------------------------------------------------------
// Functions
//! @cond DOXYGEN_SKIP
/// Pointer to user call-back function
void (*USBXCORE_apiCallback)(void);
//! @endcond

uint32_t USBX_getCallbackSource(void)
{
  uint32_t temp = USBXCORE_apiIntValue;

  USBXCORE_apiIntValue = 0;
  return temp;
}

/**************************************************************************//**
 * @brief Resets internal USBXpress variables.
 *
 * This function resets the internal USBXpress variables. It should be called
 * after a cancel-type event (USBXpress Close, USB de-configure, FIFO flush,
 * etc.).
 *****************************************************************************/
void USBXCORE_resetState(void)
{
  USBXCORE_readSize = 0;
  USBXCORE_writeSize = 0;
  USBXCORE_zlpActive = false;
  USBXCORE_rxOverflowPacketAvailable = false;
  USBXCORE_rxOverflowPacketSize = 0;
}

void USBX_setCallBack(USBX_apiCallback_t fun)
{
  USBXCORE_apiCallback = fun;       // Save API call-back pointer
  USBXCORE_apiEa |= APIEA_GIE;    // Set USBXCORE_API_EA.0 to enable
                                // USB API call-back generation
}

/**************************************************************************//**
 * @brief   Conditionally jumps to the user call-back routine
 *
 * If the user call-back routine is already in progress, this function does
 * nothing. Otherwise, this function jumps to the call-back defined via
 * USBX_callbackEnable().
 *****************************************************************************/
void USBX_jumpCallback(void)
{
  (*USBXCORE_apiCallback)();
}

/**************************************************************************//**
 * @brief   USB Reset call-back
 *
 * Jump to user API RESET call-back.
 *
 *****************************************************************************/
void USBX_ResetCb(void)
{
  USBXCORE_apiIntValue = USBX_RESET;
  USBXCORE_resetState();

  if ((USBXCORE_apiEa & APIEA_GIE) && (USBXCORE_apiIntValue)) {
    // Jump to API ISR
    USBX_jumpCallback();
  }
}

/**************************************************************************//**
 * @brief   USB device state change call-back
 *
 * Set new state and jump to user API call-back.
 *
 *****************************************************************************/
void USBX_DeviceStateChangeCb(USBD_State_TypeDef oldState, USBD_State_TypeDef newState)
{
  DBGPRINTLN("USBX_DeviceStateChangeCb %d", newState);
  (void) oldState;    // Suppress compiler warning: unused parameter

  // Entering suspend mode, power internal and external blocks down
  if(newState == USBD_STATE_SUSPENDED)
  {
    USBXCORE_apiIntValue |= USBX_DEV_SUSPEND;
    USBXCORE_resetState();
  }
  if(newState == USBD_STATE_CONFIGURED)
  {
    USBXCORE_apiIntValue |= USBX_DEV_CONFIGURED;
  }
  if(newState < USBD_STATE_CONFIGURED)
  {
    USBXCORE_resetState();
  }

  if ((USBXCORE_apiEa & APIEA_GIE) && (USBXCORE_apiIntValue)) {
    // Call to assembly function to cleanup stack and jump to API ISR
    DBGPRINTLN("USBX_outXferCompleteCb jump callback");
    USBX_jumpCallback();
  }
}

/**************************************************************************//**
 * @brief   USB setup command call-back
 *
 * If the setup command is a vendor request, pass to the USB command request
 * parsing routine and acknowledge.  Otherwise ignore the request.
 *
 *****************************************************************************/
int USBX_SetupCmdCb(const USB_Setup_TypeDef *setup)
{
  DBGPRINTLN("Setup cmd");
  DBGPRINTLN("type %d, bRequest %d, wValue %d", setup->Type, setup->bRequest, setup->wValue);
  USB_Status_TypeDef retval = USB_STATUS_REQ_UNHANDLED;
  uint16_t length;

  // Handle open and close events
  if(setup->Type == USB_SETUP_TYPE_VENDOR)
  {
    DBGPRINTLN("USB_SETUP_TYPE_VENDOR");
    // Look for vendor-specific requests
    switch(setup->bRequest)
    {
      // Requests directed to a USBXpress Device
      case SI_USBXPRESS_REQUEST:
        DBGPRINTLN("SI_USBXPRESS_REQUEST");
        switch(setup->wValue)
        {
          // Flush Buffers
          case SI_USBXPRESS_FLUSH_BUFFERS:
            DBGPRINTLN("SI_USBXPRESS_FLUSH_BUFFERS");
            if(USBXCORE_apiEa & APIEA_GIE)
            {
              USBXCORE_apiEa &= ~APIEA_GIE;      // Turn off bit 1
              USBXCORE_apiEa |= APIEA_GIE_TEMP;   // Turn on bit 2
            }
            else
            {
              USBXCORE_apiEa &= ~APIEA_GIE_TEMP;  // Turn off bit 2
            }

            // Abort the current write transfer.
            // This will flush any data in the FIFO.
            USBD_AbortTransfer(USBXPRESS_IN_EP_ADDR);

            USBXCORE_resetState();

            // Clear all other interrupts, set flush buffer interrupt
            USBXCORE_apiIntValue = USBX_FIFO_PURGE;

            if(USBXCORE_apiEa & APIEA_GIE_TEMP)
            {
              USBXCORE_apiEa |= APIEA_GIE;
            }
            retval = USB_STATUS_OK;
            break;

          // Enable
          case SI_USBXPRESS_CLEAR_TO_SEND:
            USBXCORE_apiIntValue |= USBX_DEV_OPEN;
            retval = USB_STATUS_OK;
            break;

          // Disable
          case SI_USBXPRESS_NOT_CLEAR_TO_SEND:
            USBXCORE_apiIntValue |= USBX_DEV_CLOSE;
            retval = USB_STATUS_OK;
            break;
        }
        break;
    }
  }

  // Jump to API ISR if a valid command was received.
  if (retval == USB_STATUS_OK) {
    if ((USBXCORE_apiEa & APIEA_GIE) && (USBXCORE_apiIntValue)) {
      // Jump to API ISR
      USBX_jumpCallback();
    }

    return retval;
  }

  return retval;
}

/**************************************************************************//**
 * @brief   USBXpress IN Endpoint Transfer Complete Callback
 *
 * Gets the number of IN bytes transferred and passes them to the user API
 * call-back.
 *
 *****************************************************************************/
int USBX_inXferCompleteCb(USB_Status_TypeDef status, uint16_t xferred, uint16_t remaining)
{
  DBGPRINTLN("USBX_inXferCompleteCb");
  (void) remaining;   // Suppress compiler warning: unused parameter

  if(status == USB_STATUS_OK)
  {
    *USBXCORE_byteCountInPtr += xferred;
    USBXCORE_writeSize -= xferred;

    // If the transfer was a multiple of the maximum packet size, send a ZLP
    // to the host to signal the end of the transfer.
    if(USBXCORE_writeSize == 0)
    {
      if((xferred) && (xferred % USB_FS_BULK_EP_MAXSIZE == 0))
      {
        USBD_Write(USBXPRESS_IN_EP_ADDR, NULL, 0, (USB_XferCompleteCb_TypeDef) USBX_inXferCompleteCb);
      }
      else
      {
        // Notify of transmit complete
        USBXCORE_apiIntValue |= USBX_TX_COMPLETE;
      }
    }
  }

  if((USBXCORE_apiEa & APIEA_GIE) && (USBXCORE_apiIntValue))
  {
    // Call to assembly function to cleanup stack and jump to API ISR
    USBX_jumpCallback();
  }

  return 0;
}

/**************************************************************************//**
 * @brief   USBXpress OUT Endpoint Transfer Complete Callback
 *
 * Gets the number of OUT bytes transferred and passes them to the user API
 * call-back.
 *
 *****************************************************************************/
int USBX_outXferCompleteCb(USB_Status_TypeDef status, uint16_t xferred, uint16_t remaining)
{
  DBGPRINTLN("USBX_outXferCompleteCb");
  (void) remaining;   // Suppress compiler warning: unused parameter

  if(status == USB_STATUS_OK)
  {
    DBGPRINTLN("xferred: %d", xferred);
    DBGPRINTLN("remaining: %d", remaining);
    DBGPRINTLN("USBXCORE_readSize: %d", USBXCORE_readSize);
    if(xferred <= USBXCORE_readSize)
    {
      USBXCORE_readSize -= xferred;
      *USBXCORE_byteCountOutPtr += xferred;
    }
    else
    {
      *USBXCORE_byteCountOutPtr += USBXCORE_readSize;
      USBXCORE_readSize = 0;
    }
    DBGPRINTLN("USBXCORE_readSize: %d", USBXCORE_readSize);

    // If the total read size is not decremented to zero, the transfer has ended.
    if(USBXCORE_readSize)
    {
      // Notify of receive complete
      USBXCORE_apiIntValue |= USBX_RX_COMPLETE;
    } // If this was a ZLP, mark USBX_RX_COMPLETE and USBX_RX_OVERRUN, if necessary
    else if(USBXCORE_zlpActive)
    {
      // Notify of receive complete
      USBXCORE_apiIntValue |= USBX_RX_COMPLETE;

      USBXCORE_zlpActive = false;

      // If we received data, notify of receive overrun
      if(xferred > 0)
      {
        USBXCORE_apiIntValue |= USBX_RX_OVERRUN;
        USBXCORE_rxOverflowPacketAvailable = true;
        USBXCORE_rxOverflowPacketSize = xferred;
      }
    }
    else
    {
      USBXCORE_zlpActive = true;
      USBD_Read(USBXPRESS_OUT_EP_ADDR, USBXCORE_overflowBuffer, 0, (USB_XferCompleteCb_TypeDef) USBX_outXferCompleteCb);
    }
  }

  if ((USBXCORE_apiEa & APIEA_GIE) && (USBXCORE_apiIntValue)) {
    // Call to assembly function to cleanup stack and jump to API ISR
    USBX_jumpCallback();
  }

  return 0;
}

int USBX_blockRead(uint8_t *block, uint32_t numBytes, uint32_t *countPtr)
{
  DBGPRINTLN("USBX_blockRead");
  uint32_t i;

  USBXCORE_byteCountOutPtr = countPtr;
  *USBXCORE_byteCountOutPtr = 0;

  // If the Rx Overflow Packet has data in it, copy that data to the buffer.
  if(USBXCORE_rxOverflowPacketAvailable)
  {
    for(i = 0; i < USBXCORE_rxOverflowPacketSize; i++)
    {
      *block = USBXCORE_overflowBuffer[i];
      block++;
    }

    USBXCORE_rxOverflowPacketAvailable = false;

    // If the amount of data in the overflow queue was less than the requested
    // amount of data, issue a read for the remaining data.
    if (((numBytes - USBXCORE_rxOverflowPacketSize) > 0)
        && (USBXCORE_rxOverflowPacketSize % USB_FS_BULK_EP_MAXSIZE) == 0) {
      *USBXCORE_byteCountOutPtr += USBXCORE_rxOverflowPacketSize;
      USBXCORE_readSize = numBytes;
      numBytes -= USBXCORE_rxOverflowPacketSize;
      USBXCORE_rxOverflowPacketSize = 0;

      return USBD_Read(USBXPRESS_OUT_EP_ADDR, block, numBytes, (USB_XferCompleteCb_TypeDef) USBX_outXferCompleteCb);
    }
    else
    {
      i = USBXCORE_rxOverflowPacketSize;
      USBXCORE_rxOverflowPacketSize = 0;
      USBXCORE_readSize = numBytes;

      return USBX_outXferCompleteCb(USB_STATUS_OK, i, 0);
    }
  }
  else
  {
    USBXCORE_readSize = numBytes;
    return USBD_Read(USBXPRESS_OUT_EP_ADDR, block, numBytes, (USB_XferCompleteCb_TypeDef) USBX_outXferCompleteCb);
  }
}

int USBX_blockWrite(uint8_t *block, uint32_t numBytes, uint32_t *countPtr)
{
  DBGPRINTLN("USBX_blockWrite");
  USBXCORE_byteCountInPtr = countPtr;
  *USBXCORE_byteCountInPtr = 0;
  USBXCORE_writeSize = numBytes;
  return USBD_Write(USBXPRESS_IN_EP_ADDR, block, numBytes, (USB_XferCompleteCb_TypeDef) USBX_inXferCompleteCb);
}