/***************************************************************************//**
 * @file
 * @brief USB descriptors for CDC Serial Port adapter example project.
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
#include "descriptors.h"

/// USB Device Descriptor
const USB_DeviceDescriptor_TypeDef USBDESC_deviceDesc  __attribute__((aligned(4))) =
{
  .bLength            = USB_DEVICE_DESCSIZE,
  .bDescriptorType    = USB_DEVICE_DESCRIPTOR,
  .bcdUSB             = 0x0200,
  .bDeviceClass       = 0,
  .bDeviceSubClass    = 0,
  .bDeviceProtocol    = 0,
  .bMaxPacketSize0    = USB_FS_CTRL_EP_MAXSIZE,
  .idVendor           = USB_VENDOR_ID,
  .idProduct          = USB_PRODUCT_ID,
  .bcdDevice          = 0x0100,
  .iManufacturer      = 1,
  .iProduct           = 2,
  .iSerialNumber      = 3,
  .bNumConfigurations = 1
};

#define CONFIG_DESCSIZE (USB_CONFIG_DESCSIZE                     \
                         + (USB_INTERFACE_DESCSIZE * 2)          \
                         + (USB_ENDPOINT_DESCSIZE * NUM_EP_USED) \
                         + USB_CDC_HEADER_FND_DESCSIZE           \
                         + USB_CDC_CALLMNG_FND_DESCSIZE          \
                         + USB_CDC_ACM_FND_DESCSIZE              \
                         + 5)  /*CDC Union Functional descriptor length*/

/// USB Configuration Descriptor
const uint8_t USBDESC_configDesc[]  __attribute__((aligned(4))) =
{
  // Configuration descriptor
  USB_CONFIG_DESCSIZE,              // bLength
  USB_CONFIG_DESCRIPTOR,            // bDescriptorType

  USB_CONFIG_DESCSIZE               // wTotalLength (LSB)
  + USB_INTERFACE_DESCSIZE
  + (USB_ENDPOINT_DESCSIZE * NUM_EP_USED),

  (USB_CONFIG_DESCSIZE              // wTotalLength (MSB)
   + USB_INTERFACE_DESCSIZE
   + (USB_ENDPOINT_DESCSIZE * NUM_EP_USED)) >> 8,

  1,                                // bNumInterfaces
  1,                                // bConfigurationValue
  0,                                // iConfiguration

#if defined(USB_BUS_POWERED)
  CONFIG_DESC_BM_RESERVED_D7,       // bmAttrib: Bus-powered
#else
  CONFIG_DESC_BM_RESERVED_D7        // bmAttrib: Self-powered
  | CONFIG_DESC_BM_SELFPOWERED,
#endif
  CONFIG_DESC_MAXPOWER_mA(200),     // bMaxPower: 200 mA

  // Interface descriptor
  USB_INTERFACE_DESCSIZE,           // bLength
  USB_INTERFACE_DESCRIPTOR,         // bDescriptorType
  0,                                // bInterfaceNumber
  0,                                // bAlternateSetting
  2,                                // bNumEndpoints
  USB_CLASS_VENDOR,                 // bInterfaceClass (Vendor-Specific)
  0,                                // bInterfaceSubClass
  0,                                // bInterfaceProtocol
  0,                                // iInterface

  // Endpoint 1 IN descriptor
  USB_ENDPOINT_DESCSIZE,            // bLength
  USB_ENDPOINT_DESCRIPTOR,          // bDescriptorType
  0x81,                             // bEndpointAddress (IN)
  USB_EPTYPE_BULK,                  // bmAttributes
  USB_FS_BULK_EP_MAXSIZE,           // wMaxPacketSize (LSB)
  0,                                // wMaxPacketSize (MSB)
  0,                                // bInterval

  // Endpoint 1 OUT descriptor
  USB_ENDPOINT_DESCSIZE,            // bLength
  USB_ENDPOINT_DESCRIPTOR,          // bDescriptorType
  0x01,                             // bEndpointAddress (OUT)
  USB_EPTYPE_BULK,                  // bmAttributes
  USB_FS_BULK_EP_MAXSIZE,           // wMaxPacketSize (LSB)
  0,                                // wMaxPacketSize (MSB)
  0,                                // bInterval
};

STATIC_CONST_STRING_DESC_LANGID(langID, 0x04, 0x09);
STATIC_CONST_STRING_DESC(iManufacturer, 'S', 'i', 'l', 'i', 'c', 'o', 'n', ' ', 'L', \
                         'a', 'b', 'o', 'r', 'a', 't', 'o', 'r', 'i',                \
                         'e', 's', ' ', 'I', 'n', 'c', '.');
STATIC_CONST_STRING_DESC(iProduct, 'E', 'F', 'M', '3', '2', ' ', 'U', 'S', 'B', \
                         ' ', 'C', 'D', 'C', ' ', 's', 'e', 'r', 'i',           \
                         'a', 'l', ' ', 'p', 'o', 'r', 't', ' ', 'd',           \
                         'e', 'v', 'i', 'c', 'e');

const void * const USBDESC_strings[] =
{
  &langID,
  &iManufacturer,
  &iProduct,
};


/* Endpoint buffer sizes */
/* 1 = single buffer, 2 = double buffering, 3 = triple buffering ...  */
/* Use double buffering on the BULK endpoints.                        */
const uint8_t USBDESC_bufferingMultiplier[NUM_EP_USED + 1] = { 1, 2, 2 };
