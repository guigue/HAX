/**
 * \file
 *
 * \brief Default descriptors for a USB Device with a single interface MSC
 *
 * Copyright (c) 2009-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "conf_usb.h"
#include "udd.h"
#include "udc_desc.h"
#include "udi_msc.h"


/**
 * \defgroup udi_msc_group_single_desc USB device descriptors for a single interface
 *
 * The following structures provide the USB device descriptors required
 * for USB Device with a single interface MSC.
 *
 * It is ready to use and do not require more definition.
 * @{
 */

//! Only one interface for this device
#define  USB_DEVICE_NB_INTERFACE       1

//! USB Device Descriptor
UDC_DATA(4)
UDC_DESC_STORAGE usb_dev_desc_t udc_device_desc = {
	.bLength                   = sizeof(usb_dev_desc_t),
	.bDescriptorType           = USB_DT_DEVICE,
	.bcdUSB                    = LE16(USB_V2_0),
	.bDeviceClass              = 0,
	.bDeviceSubClass           = 0,
	.bDeviceProtocol           = 0,
	.bMaxPacketSize0           = USB_DEVICE_EP_CTRL_SIZE,
	.idVendor                  = LE16(USB_DEVICE_VENDOR_ID),
	.idProduct                 = LE16(USB_DEVICE_PRODUCT_ID),
	.bcdDevice                 = LE16((USB_DEVICE_MAJOR_VERSION << 8)
		| USB_DEVICE_MINOR_VERSION),
#ifdef USB_DEVICE_MANUFACTURE_NAME
	.iManufacturer             = 1,
#else
	.iManufacturer             = 0,  // No manufacture string
#endif
#ifdef USB_DEVICE_PRODUCT_NAME
	.iProduct                  = 2,
#else
	.iProduct                  = 0,  // No product string
#endif
#if (defined USB_DEVICE_SERIAL_NAME || defined USB_DEVICE_GET_SERIAL_NAME_POINTER)
	.iSerialNumber             = 3,
#else
	.iSerialNumber             = 0,  // No serial string
#endif
	.bNumConfigurations        = 1
};


#ifdef USB_DEVICE_HS_SUPPORT
//! USB Device Qualifier Descriptor for HS
UDC_DATA(4)
UDC_DESC_STORAGE usb_dev_qual_desc_t udc_device_qual = {
	.bLength                   = sizeof(usb_dev_qual_desc_t),
	.bDescriptorType           = USB_DT_DEVICE_QUALIFIER,
	.bcdUSB                    = LE16(USB_V2_0),
	.bDeviceClass              = 0,
	.bDeviceSubClass           = 0,
	.bDeviceProtocol           = 0,
	.bMaxPacketSize0           = USB_DEVICE_EP_CTRL_SIZE,
	.bNumConfigurations        = 1
};
#endif

//! Structure for USB Device Configuration Descriptor
COMPILER_PACK_SET(1)
typedef struct {
	usb_conf_desc_t conf;
	udi_msc_desc_t udi_msc;
} udc_desc_t;
COMPILER_PACK_RESET()

//! USB Device Configuration Descriptor filled for FS
UDC_DATA(4)
UDC_DESC_STORAGE udc_desc_t udc_desc_fs = {
	.conf.bLength              = sizeof(usb_conf_desc_t),
	.conf.bDescriptorType      = USB_DT_CONFIGURATION,
	.conf.wTotalLength         = LE16(sizeof(udc_desc_t)),
	.conf.bNumInterfaces       = USB_DEVICE_NB_INTERFACE,
	.conf.bConfigurationValue  = 1,
	.conf.iConfiguration       = 0,
	.conf.bmAttributes         = USB_CONFIG_ATTR_MUST_SET | USB_DEVICE_ATTR,
	.conf.bMaxPower            = USB_CONFIG_MAX_POWER(USB_DEVICE_POWER),
	.udi_msc                   = UDI_MSC_DESC_FS,
};

#ifdef USB_DEVICE_HS_SUPPORT
//! USB Device Configuration Descriptor filled for HS
UDC_DATA(4)
UDC_DESC_STORAGE udc_desc_t udc_desc_hs = {
	.conf.bLength              = sizeof(usb_conf_desc_t),
	.conf.bDescriptorType      = USB_DT_CONFIGURATION,
	.conf.wTotalLength         = LE16(sizeof(udc_desc_t)),
	.conf.bNumInterfaces       = USB_DEVICE_NB_INTERFACE,
	.conf.bConfigurationValue  = 1,
	.conf.iConfiguration       = 0,
	.conf.bmAttributes         = USB_CONFIG_ATTR_MUST_SET | USB_DEVICE_ATTR,
	.conf.bMaxPower            = USB_CONFIG_MAX_POWER(USB_DEVICE_POWER),
	.udi_msc                   = UDI_MSC_DESC_HS,
};
#endif


/**
 * \name UDC structures which contains all USB Device definitions
 */
//@{

//! Associate an UDI for each USB interface
UDC_DESC_STORAGE udi_api_t *udi_apis[USB_DEVICE_NB_INTERFACE] = {
	&udi_api_msc,
};

//! Add UDI with USB Descriptors FS
UDC_DESC_STORAGE udc_config_speed_t   udc_config_lsfs[1] = {{
	.desc          = (usb_conf_desc_t UDC_DESC_STORAGE*)&udc_desc_fs,
	.udi_apis      = udi_apis,
}};

#ifdef USB_DEVICE_HS_SUPPORT
//! Add UDI with USB Descriptors HS
UDC_DESC_STORAGE udc_config_speed_t   udc_config_hs[1] = {{
	.desc          = (usb_conf_desc_t UDC_DESC_STORAGE*)&udc_desc_hs,
	.udi_apis      = udi_apis,
}};
#endif

//! Add all information about USB Device in global structure for UDC
UDC_DESC_STORAGE udc_config_t udc_config = {
	.confdev_lsfs = &udc_device_desc,
	.conf_lsfs = udc_config_lsfs,
#ifdef USB_DEVICE_HS_SUPPORT
	.confdev_hs = &udc_device_desc,
	.qualifier = &udc_device_qual,
	.conf_hs = udc_config_hs,
#endif
};

//@}
//@}
