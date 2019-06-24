/***************************************************************************//**
 * @file
 * @brief Gecko USB Communication Device Class (CDC) driver.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "em_device.h"
#include "debug_macros.h"
#include "usb_common_tmp.h"
#include "usb.h"
#include "cdc.h"

/* *INDENT-OFF* */

/***************************************************************************//**
 * @addtogroup kitdrv
 * @{
 ******************************************************************************/

/**************************************************************************//**
 * @addtogroup gusbcdc Gecko USB RS232 bridge
 * @brief Gecko USB to RS232 bridge (CDC Class).
 *
 * @details

   @section cdc_intro CDC Implementation

   This driver implements a basic USB to RS232 bridge.

   @section cdc_config CDC Device Configuration Options

   This section contains a description of the configuration options for
   the driver. The options are @htmlonly #defines @endhtmlonly which are
   expected to be found in the application "usbconfig.h" header file.
   The values shown below are from the Giant Gecko DK3750 CDC example.

   @verbatim
 // USB interface numbers. Interfaces are numbered from zero to one less than
 // the number of concurrent interfaces supported by the configuration. A CDC
 // device is by itself a composite device and has two interfaces.
 // The interface numbers must be 0 and 1 for a standalone CDC device, for a
 // composite device which includes a CDC interface it must not be in conflict
 // with other device interfaces.
 #define CDC_CTRL_INTERFACE_NO ( 0 )
 #define CDC_DATA_INTERFACE_NO ( 1 )

 // Endpoint address for CDC data reception.
 #define CDC_EP_DATA_OUT ( 0x01 )

 // Endpoint address for CDC data transmission.
 #define CDC_EP_DATA_IN ( 0x81 )

 // Endpoint address for the notification endpoint (not used).
 #define CDC_EP_NOTIFY ( 0x82 )

 // Timer id, see USBTIMER in the USB device stack documentation.
 // The CDC driver has a Rx timeout functionality which require a timer.
 #define CDC_TIMER_ID ( 0 )

 // DMA related macros, select DMA channels and DMA request signals.
 #define CDC_UART_TX_DMA_CHANNEL   ( 0 )
 #define CDC_UART_RX_DMA_CHANNEL   ( 1 )
 #define CDC_TX_DMA_SIGNAL         DMAREQ_UART1_TXBL
 #define CDC_RX_DMA_SIGNAL         DMAREQ_UART1_RXDATAV

 // UART/USART selection macros.
 #define CDC_UART                  UART1
 #define CDC_UART_CLOCK            cmuClock_UART1
 #define CDC_UART_ROUTE            ( UART_ROUTE_RXPEN | UART_ROUTE_TXPEN | \
                                    UART_ROUTE_LOCATION_LOC2 )
 #define CDC_UART_TX_PORT          gpioPortB
 #define CDC_UART_TX_PIN           9
 #define CDC_UART_RX_PORT          gpioPortB
 #define CDC_UART_RX_PIN           10

 // Activate the RS232 switch on DKs.
 #define CDC_ENABLE_DK_UART_SWITCH() BSP_PeripheralAccess(BSP_RS232_UART, true)

 // No RS232 switch on STKs. Leave the definition "empty".
 #define CDC_ENABLE_DK_UART_SWITCH()

   @endverbatim
 * @{
 ******************************************************************************/

/* *INDENT-ON* */

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/*** Typedes and defines ***/

#define CDC_BULK_EP_SIZE  (USB_FS_BULK_EP_MAXSIZE) /* This is the max. ep size.    */
#define CDC_USB_RX_BUF_SIZ  CDC_BULK_EP_SIZE /* Packet size when receiving on USB. */
#define CDC_USB_TX_BUF_SIZ  127    /* Packet size when transmitting on USB.  */

/* Calculate a timeout in ms corresponding to 5 char times on current     */
/* baudrate. Minimum timeout is set to 10 ms.                             */
#define CDC_RX_TIMEOUT    SL_MAX(10U, 50000 / (cdcLineCoding.dwDTERate))

/* The serial port LINE CODING data structure, used to carry information  */
/* about serial port baudrate, parity etc. between host and device.       */
SL_PACK_START(1)
typedef struct {
  uint32_t dwDTERate;               /** Baudrate                            */
  uint8_t  bCharFormat;             /** Stop bits, 0=1 1=1.5 2=2            */
  uint8_t  bParityType;             /** 0=None 1=Odd 2=Even 3=Mark 4=Space  */
  uint8_t  bDataBits;               /** 5, 6, 7, 8 or 16                    */
  uint8_t  dummy;                   /** To ensure size is a multiple of 4 bytes */
} SL_ATTRIBUTE_PACKED cdcLineCoding_TypeDef;
SL_PACK_END()

/*** Function prototypes ***/


/*** Variables ***/

/*
 * The LineCoding variable must be 4-byte aligned as it is used as USB
 * transmit and receive buffer.
 */
SL_ALIGN(4)
SL_PACK_START(1)
static cdcLineCoding_TypeDef SL_ATTRIBUTE_ALIGN(4) cdcLineCoding =
{
  115200, 0, 0, 8, 0
};
SL_PACK_END()

STATIC_UBUF(usbRxBuffer0, CDC_USB_RX_BUF_SIZ);    /* USB receive buffers.   */
STATIC_UBUF(usbRxBuffer1, CDC_USB_RX_BUF_SIZ);
STATIC_UBUF(uartRxBuffer0, CDC_USB_TX_BUF_SIZ);   /* UART receive buffers.  */
STATIC_UBUF(uartRxBuffer1, CDC_USB_TX_BUF_SIZ);

static const uint8_t  *usbRxBuffer[2] = { usbRxBuffer0, usbRxBuffer1 };
static const uint8_t  *uartRxBuffer[2] = { uartRxBuffer0, uartRxBuffer1 };

static int            usbRxIndex, usbBytesReceived;
static int            uartRxIndex, uartRxCount;
static int            LastUsbTxCnt;

static bool           dmaRxCompleted;
static bool           usbRxActive, dmaTxActive;
static bool           usbTxActive, dmaRxActive;

//static DMA_CB_TypeDef DmaTxCallBack;    /** DMA callback structures. */
//static DMA_CB_TypeDef DmaRxCallBack;

/** @endcond */

/**************************************************************************//**
 * @brief CDC device initialization.
 *****************************************************************************/
void CDC_Init(void)
{
  //SerialPortInit();
  //DmaSetup();
}

/**************************************************************************//**
 * @brief
 *   Handle USB setup commands. Implements CDC class specific commands.
 *
 * @param[in] setup Pointer to the setup packet received.
 *
 * @return USB_STATUS_OK if command accepted.
 *         USB_STATUS_REQ_UNHANDLED when command is unknown, the USB device
 *         stack will handle the request.
 *****************************************************************************/
int CDC_SetupCmd(const USB_Setup_TypeDef *setup)
{
  int retVal = USB_STATUS_REQ_UNHANDLED;

  if((setup->Type == USB_SETUP_TYPE_CLASS) && (setup->Recipient == USB_SETUP_RECIPIENT_INTERFACE))
  {
    switch(setup->bRequest)
    {
      case USB_CDC_GETLINECODING:
        /********************/
        if((setup->wValue         == 0)
             && (setup->wIndex    == CDC_CTRL_INTERFACE_NO) /* Interface no. */
             && (setup->wLength   == 7)                     /* Length of cdcLineCoding. */
             && (setup->Direction == USB_SETUP_DIR_IN)    ) {
          /* Send current settings to USB host. */
          DBGPRINTLN("Send current settings to USB host.");
          USBD_Write(0, (void*) &cdcLineCoding, 7, NULL);
          retVal = USB_STATUS_OK;
        }
        break;

      case USB_CDC_SETLINECODING:
        /********************/
        if ( (setup->wValue       == 0)
             && (setup->wIndex    == CDC_CTRL_INTERFACE_NO) /* Interface no. */
             && (setup->wLength   == 7)                     /* Length of cdcLineCoding. */
             && (setup->Direction != USB_SETUP_DIR_IN)    ) {
          DBGPRINTLN("Get new settings from USB host.");
          /* Get new settings from USB host. */
          USBD_Read(0, (void*) &cdcLineCoding, 7, NULL);
          retVal = USB_STATUS_OK;
        }
        break;

      case USB_CDC_SETCTRLLINESTATE:
        /********************/
        if ( (setup->wIndex     == CDC_CTRL_INTERFACE_NO)      /* Interface no.  */
             && (setup->wLength == 0)    ) {                /* No data.       */
          DBGPRINTLN("Do nothing ( Non compliant behaviour !! )");
          /* Do nothing ( Non compliant behaviour !! ) */
          retVal = USB_STATUS_OK;
        }
        break;
    }
  }

  return retVal;
}

/**************************************************************************//**
 * @brief
 *   Callback function called each time the USB device state is changed.
 *   Starts CDC operation when device has been configured by USB host.
 *
 * @param[in] oldState The device state the device has just left.
 * @param[in] newState The new device state.
 *****************************************************************************/
void CDC_StateChangeEvent(USBD_State_TypeDef oldState,
                          USBD_State_TypeDef newState)
{
  if (newState == USBD_STATE_CONFIGURED) {
    /* We have been configured, start CDC functionality ! */
    DBGPRINTLN("We have been configured, start CDC functionality !");

    if (oldState == USBD_STATE_SUSPENDED) { /* Resume ?   */
    }

    /* Start receiving data from USB host. */
    DBGPRINTLN("Start receiving data from USB host.");

    /* Start receiving data on UART. */
  } else if ((oldState == USBD_STATE_CONFIGURED) && (newState != USBD_STATE_SUSPENDED)) {
    /* We have been de-configured, stop CDC functionality. */
    /* Stop DMA channels. */
  } else if (newState == USBD_STATE_SUSPENDED) {
    /* We have been suspended, stop CDC functionality. */
    DBGPRINTLN(" We have been suspended, stop CDC functionality.");
  }
}