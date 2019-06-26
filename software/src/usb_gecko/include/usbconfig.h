/***************************************************************************//**
 * @file
 * @brief   USB protocol stack library, application supplied configuration
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

#ifndef SILICON_LABS_USBCONFIG_H
#define SILICON_LABS_USBCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#define USB_DEVICE        // Compile stack for device mode.

#define USB_CLKSRC_USHFRCO // Use HFRCO as core clock, default is HFXO.

//#define USB_PWRSAVE_MODE (USB_PWRSAVE_MODE_ONVBUSOFF   \
//                          | USB_PWRSAVE_MODE_ONSUSPEND \
//                          | USB_PWRSAVE_MODE_ENTEREM2)

#define DEBUG_USB_API
#define DEBUG_USB_INT_LO
#define DEBUG_USB_INT_HI

/****************************************************************************
**                                                                         **
** Specify number of endpoints used (in addition to EP0).                  **
**                                                                         **
*****************************************************************************/
#define NUM_EP_USED 2

/****************************************************************************
**                                                                         **
** Specify number of application timers you need.                          **
**                                                                         **
*****************************************************************************/
#define NUM_APP_TIMERS 1

#define USBX_RESET               0x00000001     //!< USB Reset Interrupt has occurred
#define USBX_TX_COMPLETE         0x00000002     //!< Transmit Complete Interrupt has occurred
#define USBX_RX_COMPLETE         0x00000004     //!< Receive Complete Interrupt has occurred
#define USBX_FIFO_PURGE          0x00000008     //!< Receive and Transmit FIFO's were purged
#define USBX_DEV_OPEN            0x00000010     //!< Device Instance Opened on host side
#define USBX_DEV_CLOSE           0x00000020     //!< Device Instance Closed on host side
#define USBX_DEV_CONFIGURED      0x00000040     //!< Device has entered configured state
#define USBX_DEV_SUSPEND         0x00000080     //!< USB suspend signaling present on bus
#define USBX_RX_OVERRUN          0x00000100     //!< Data received with no place to put it

#define USBX_STATUS_OK                  USB_STATUS_OK                   //!< Success
#define USBX_STATUS_EP_BUSY             USB_STATUS_EP_BUSY              //!< Failed because the endpoint is busy
#define USBX_STATUS_ILLEGAL             USB_STATUS_ILLEGAL              //!< Failed due to an illegal parameter
#define USBX_STATUS_EP_STALLED          USB_STATUS_EP_STALLED           //!< Failed because the endpoint is stalled
#define USBX_STATUS_DEVICE_UNCONFIGURED USB_STATUS_DEVICE_UNCONFIGURED  //!< Failed because the device is not configured

// Control command requests and values
#define SI_USBXPRESS_REQUEST            0x02    //!< Request
#define SI_USBXPRESS_FLUSH_BUFFERS      0x0001  //!< Value
#define SI_USBXPRESS_CLEAR_TO_SEND      0x0002  //!< Value
#define SI_USBXPRESS_NOT_CLEAR_TO_SEND  0x0004  //!< Value

#ifdef __cplusplus
}
#endif

#endif // SILICON_LABS_USBCONFIG_H
