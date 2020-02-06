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

// Control command requests and values
#define SI_USBXPRESS_REQUEST            0x02    //!< Request
#define SI_USBXPRESS_FLUSH_BUFFERS      0x01  //!< Value
#define SI_USBXPRESS_CLEAR_TO_SEND      0x02  //!< Value
#define SI_USBXPRESS_NOT_CLEAR_TO_SEND  0x04  //!< Value

#ifdef __cplusplus
}
#endif

#endif // SILICON_LABS_USBCONFIG_H