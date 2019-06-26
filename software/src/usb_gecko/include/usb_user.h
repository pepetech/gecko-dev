/***************************************************************************//**
 * @file
 * @brief   Header file for the USBXpress firmware library.  Includes function
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

/**************************************************************************//**
 * @cond DOXYDOC_USB_PRESENT
 * @addtogroup usbxpress USBXpress
 * @{
 *
 * @brief
 *   USBXpress interface library.
 *
 * ## Introduction
 *
 *   This module provides a firmware interface which communicates with the
 *   Silicon Labs USBXpress driver and allows firmware to interact with host
 *   programs via the USBXpress DLL.
 *
 * ## Theory of operation:
 *
 *   The USBxpress library interfaces with the Silicon Labs USBXpress drivers.
 *   The interface consists of two data transmission pipes (RX and TX) which
 *   are used to send and receive data.
 *
 *  ### Data
 *
 *   Data reception and transmission is handled with USBX_blockRead() and
 *   USBX_blockWrite(). Upon completion of a sent or received transfer the user
 *   is called back with USBX_RX_COMPLETE or USBX_TX_COMPLETE. If the user
 *   calls USBX_blockRead() with a numBytes value smaller than the number of
 *   bytes received in the transfer, the user is called back with both
 *   USBX_RX_COMPLETE and USBX_RX_OVERRUN set.
 *
 *  # Additional Resources
 *
 *   * [USBXpress AppNote - AN169](http://www.silabs.com/Support%20Documents/TechnicalDocs/AN169.pdf)
 *   * [Development drivers](http://www.silabs.com/products/mcu/Pages/USBXpress.aspx)
 *   * [Driver Customization - AN220](http://www.silabs.com/Support%20Documents/TechnicalDocs/AN200.pdf)
 *
 *   This driver sits on top of the EFM32 USB Library.
 *
 * ## Interrupts
 *
 *   This library handles USB interrupts and will enable USB interrupts at
 *   its  discretion. It will NOT enable global interrupts and the user is
 *   responsible for enabling global interrupts.
 * @endcond
 *****************************************************************************/

#ifndef  EM_USBXPRESS_H
#define  EM_USBXPRESS_H

#include <stdint.h>
#include <stdbool.h>

#include "debug_macros.h"
#include "usb_common_tmp.h"
#include "usb.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * Define user API call-back function with:
 *
 * void my_callback(void) {}
 *
 * then pass it to USBX_apiCallbackEnable() with:
 *
 * USBX_apiCallbackEnable(my_callback);
 *
 * @warn
 *   The call-back routine is called by the USBXpress library from
 *   within the USB interrupt. It should run as quickly as possible.
 *
 * If timing becomes an issue, consider using the call-back routine to set
 * user-defined flags that may be serviced in the project's main loop.
 * Additionally note that the call-back routine will be called for every
 * call-back source, whether or not the user checks for that call-back source.
 * All code except variable declarations and the USBX_getCallbackSource() call
 * should check for a pertinent call-back source (see the @ref usbx_status
 * section for all call-back sources).
 *
 *****************************************************************************/
typedef void (*USBX_apiCallback_t)(void);

/***************************************************************************//**
 *  @cond DOXYDOC_USB_PRESENT
 *  @addtogroup usbx_callback_status USBXpress Callback Status Flags
 *  @{
 *
 *  These constant values are returned by USBX_getCallbackSource(). The return
 *  value is a mask and may contain the logical OR of any combination of status
 *  flags. Each flag maps to a call-back event.
 *
 ******************************************************************************/
// Basic USBX_getCallbackSource() return value bit masks (32 bits)
// Note: More than one bit can be set at the same time.
#define USBX_RESET               0x00000001     //!< USB Reset Interrupt has occurred
#define USBX_TX_COMPLETE         0x00000002     //!< Transmit Complete Interrupt has occurred
#define USBX_RX_COMPLETE         0x00000004     //!< Receive Complete Interrupt has occurred
#define USBX_FIFO_PURGE          0x00000008     //!< Receive and Transmit FIFO's were purged
#define USBX_DEV_OPEN            0x00000010     //!< Device Instance Opened on host side
#define USBX_DEV_CLOSE           0x00000020     //!< Device Instance Closed on host side
#define USBX_DEV_CONFIGURED      0x00000040     //!< Device has entered configured state
#define USBX_DEV_SUSPEND         0x00000080     //!< USB suspend signaling present on bus
#define USBX_RX_OVERRUN          0x00000100     //!< Data received with no place to put it
/**  @} (end usbx_callback_status Status Flags) */
/**  @endcond */

/***************************************************************************//**
 *  @cond DOXYDOC_USB_PRESENT
 *  @addtogroup usbx_status USB Status Flags
 *  @{
 *
 *  These constant values are returned by USBX_blockRead() and
 *  USBX_blockWrite().
 *
 ******************************************************************************/
#define USBX_STATUS_OK                  USB_STATUS_OK                   //!< Success
#define USBX_STATUS_EP_BUSY             USB_STATUS_EP_BUSY              //!< Failed because the endpoint is busy
#define USBX_STATUS_ILLEGAL             USB_STATUS_ILLEGAL              //!< Failed due to an illegal parameter
#define USBX_STATUS_EP_STALLED          USB_STATUS_EP_STALLED           //!< Failed because the endpoint is stalled
#define USBX_STATUS_DEVICE_UNCONFIGURED USB_STATUS_DEVICE_UNCONFIGURED  //!< Failed because the device is not configured
/**  @} (end usbx_status Status Flags) */
/**  @endcond */

// Define USBXCORE_apiEa bit masks
#define  APIEA_GIE                     0x01     /**< API Global Interrupt
                                                   Enable */
#define  APIEA_GIE_TEMP                0x02     /**< Temporary storage of API
                                                   GIE bit when internally
                                                   disabled */

// Control command requests and values
#define SI_USBXPRESS_REQUEST            0x02    //!< Request
#define SI_USBXPRESS_FLUSH_BUFFERS      0x0001  //!< Value
#define SI_USBXPRESS_CLEAR_TO_SEND      0x0002  //!< Value
#define SI_USBXPRESS_NOT_CLEAR_TO_SEND  0x0004  //!< Value


/**************************************************************************//**
 * @brief
 *   User API function to send data to host.
 *
 * @param   block:
 *   Pointer to user's array where data to be transmitted is stored.
 * @param   numBytes:
 *   Number of bytes to send to host
 * @param[out]   countPtr:
 *   Pointer to user's storage for number of bytes actually transmitted to the
 *   host.  This will be valid upon the subsequent USBX_TX_COMPLETE call-back.
 *
 * @return
 *   This function returns a status word. 0 indicates that the write was
 *   successfully started. Any other value indicates that the request was
 *   ignored. The most common reason is USBX_STATUS_EP_BUSY meaning that
 *   a write is already in progress.
 *
 *   Valid return codes are fond in @ref usbx_status
 *
 * A USBX_TX_COMPLETE call-back will occur when the entire transfer has
 * completed. For example if the user writes a block of 100 bytes, two packets
 * will be sent (one 64 byte packet and one 36 byte packet), but the
 * USBX_TX_COMPLETE call-back will only occur after the final packet has
 * been sent.
 *
 * This function utilizes the EFM32 USB Library's USBD_Write() function, which
 * only prepares for a write.  The actual write will occur over time as the
 * host transmits data.
 *
 *****************************************************************************/
int USBX_blockWrite(uint8_t* block, uint32_t numBytes, uint32_t* countPtr);

/**************************************************************************//**
 * @brief
 *   User API function to get data from host.
 *
 * @param   block:
 *   Pointer to user's array where received data will be stored.
 * @param   numBytes:
 *   Number of bytes to receive from host
 * @param[out]   countPtr:
 *   Pointer to user's storage for number of bytes actually received from the
 *   host.  This will be valid upon the subsequent USBX_RX_COMPLETE call-back.
 *
 * @return
 *   This function returns a status word. 0 indicates that the read was
 *   successfully started. Any other value indicates that the request was
 *   ignored. The most common reason is USBX_STATUS_EP_BUSY meaning that
 *   a read is already in progress.
 *
 *   Valid return codes are fond in @ref usbx_status
 *
 * A USBX_RX_COMPLETE call-back will occur when the read transfer has completed.
 * For example if the user reads a block of 100 bytes, two packets will be
 * received (one 64 byte packet and one 36 byte packet), but the
 * USBX_RX_COMPLETE call-back will only occur after the final packet has been
 * received.
 *
 * If the device receives more data than the value specified by numBytes, the
 * USBX_RX_COMPLETE call-back will occur when numBytes number of bytes has been
 * received. At this point, the USBX_RX_OVERRUN status flag will also be set,
 * indicating that more data was received than is available to write to the
 * buffer at block. At this point, the application may call USBX_blockRead()
 * again to receive the remaining data from the transfer.
 *
 * This function utilizes the EFM32 USB Library's USBD_Read() function,
 * which only prepares for a read.  The actual read will occur over time as the
 * host transmits data.
 *
 *****************************************************************************/
int USBX_blockRead(uint8_t* block, uint32_t numBytes, uint32_t* countPtr);

/**************************************************************************//**
 * @brief
 *   User API function to get the call-back source.
 *
 * Returns an 32-bit value indicating the reason(s) for the API call-back, and
 * clears the USBXpress API call-back pending flag(s). This function should be
 * called at the beginning of the user's call-back service routine to determine
 * which event(s) has/have occurred.
 *
 * @return
 *   An unsigned 32-bit code indicating the reason(s) for the API
 *   interrupt. The code can indicate more than one type of interrupt at
 *   the same time. Valid flags are found in @ref usbx_status
 *
 *****************************************************************************/
uint32_t USBX_getCallbackSource(void);

void USBX_setCallBack(USBX_apiCallback_t);

void USBX_ResetCb(void);
/**************************************************************************//**
 * @brief   USB device state change call-back
 *
 * Set new state and jump to user API call-back.
 *
 *****************************************************************************/
void USBX_DeviceStateChangeCb(USBD_State_TypeDef oldState, USBD_State_TypeDef newState);

/**************************************************************************//**
 * @brief   USB setup command call-back
 *
 * If the setup command is a vendor request, pass to the USB command request
 * parsing routine and acknowledge.  Otherwise ignore the request.
 *
 *****************************************************************************/
int USBX_SetupCmdCb(const USB_Setup_TypeDef *setup);

/**  @} (end usbx_func Functions) */

/**  @} (end addtogroup usbxpress USBXpress) */
/**  @endcond */

#ifdef __cplusplus
}
#endif

#endif  // EM_USBXPRESS_H
