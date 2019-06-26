/***************************************************************************//**
 * @file
 * @brief USB descriptor prototypes for CDC Serial Port adapter example project.
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
#ifndef __SILICON_LABS_DESCRIPTORS_H__
#define __SILICON_LABS_DESCRIPTORS_H__

#include "usb.h"

// USB Identification
// Initial VID/PID values.  Should never be used.  VID/PID loaded by USB_Init()
#define USB_VENDOR_ID               SILABS_USB_VID
#define USB_PRODUCT_ID              0x0000

// Interface number for USBXpress
#define USBXPRESS_IFC_NUMBER        0

#define USBXPRESS_SETUP_EP_ADDR     0x00
#define USBXPRESS_IN_EP_ADDR        0x81
#define USBXPRESS_OUT_EP_ADDR       0x01

#ifdef __cplusplus
extern "C" {
#endif

extern const USB_DeviceDescriptor_TypeDef   USBDESC_deviceDesc;
extern const uint8_t                        USBDESC_configDesc[];
extern const void * const                   USBDESC_strings[3];
extern const uint8_t                        USBDESC_bufferingMultiplier[];

#ifdef __cplusplus
}
#endif

#endif /* __SILICON_LABS_DESCRIPTORS_H__ */
