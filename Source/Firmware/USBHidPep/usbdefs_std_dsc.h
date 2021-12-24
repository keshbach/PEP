/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

/******************************************************************************
 * USB Definitions: Standard Descriptors
 *****************************************************************************/
#if !defined(USBDEFS_STD_DSC_H)
#define USBDEFS_STD_DSC_H

#include "UtTypeDefs.h"

/******************************************************************************
 * USB Endpoint Definitions
 * USB Standard EP Address Format: DIR:X:X:X:EP3:EP2:EP1:EP0
 * This is used in the descriptors. See autofiles\usbdsc.c
 * 
 * NOTE: Do not use these values for checking against USTAT.
 * To check against USTAT, use values defined in "system\usb\usbdrv\usbdrv.h"
 *****************************************************************************/
#define _EP01_OUT   0x01
#define _EP01_IN    0x81

/* Configuration Attributes */
#define _DEFAULT    0x01<<7         //Default Value (Bit 7 is set)
#define _SELF       0x01<<6         //Self-powered (Supports if set)
#define _RWU        0x01<<5         //Remote Wakeup (Supports if set)

/* Endpoint Transfer Type */
#define _CTRL       0x00            //Control Transfer
#define _ISO        0x01            //Isochronous Transfer
#define _BULK       0x02            //Bulk Transfer
#define _INT        0x03            //Interrupt Transfer

/* Isochronous Endpoint Synchronization Type */
#define _NS         0x00<<2         //No Synchronization
#define _AS         0x01<<2         //Asynchronous
#define _AD         0x02<<2         //Adaptive
#define _SY         0x03<<2         //Synchronous

/* Isochronous Endpoint Usage Type */
#define _DE         0x00<<4         //Data endpoint
#define _FE         0x01<<4         //Feedback endpoint
#define _IE         0x02<<4         //Implicit feedback Data endpoint


/** S T R U C T U R E ********************************************************/

/******************************************************************************
 * USB Device Descriptor Structure
 *****************************************************************************/
typedef struct _USB_DEV_DSC
{
    UINT8  bLength;       UINT8  bDscType;    UINT16 bcdUSB;
    UINT8  bDevCls;       UINT8  bDevSubCls;  UINT8  bDevProtocol;
    UINT8  bMaxPktSize0;  UINT16 idVendor;    UINT16 idProduct;
    UINT16 bcdDevice;     UINT8  iMFR;        UINT8  iProduct;
    UINT8  iSerialNum;    UINT8  bNumCfg;
} USB_DEV_DSC;

/******************************************************************************
 * USB Configuration Descriptor Structure
 *****************************************************************************/
typedef struct _USB_CFG_DSC
{
    UINT8 bLength;       UINT8 bDscType;      UINT16 wTotalLength;
    UINT8 bNumIntf;      UINT8 bCfgValue;     UINT8 iCfg;
    UINT8 bmAttributes;  UINT8 bMaxPower;
} USB_CFG_DSC;

/******************************************************************************
 * USB Interface Descriptor Structure
 *****************************************************************************/
typedef struct _USB_INTF_DSC
{
    UINT8 bLength;       UINT8 bDscType;      UINT8 bIntfNum;
    UINT8 bAltSetting;   UINT8 bNumEPs;       UINT8 bIntfCls;
    UINT8 bIntfSubCls;   UINT8 bIntfProtocol; UINT8 iIntf;
} USB_INTF_DSC;

/******************************************************************************
 * USB Endpoint Descriptor Structure
 *****************************************************************************/
typedef struct _USB_EP_DSC
{
    UINT8 bLength;       UINT8  bDscType;      UINT8 bEPAdr;
    UINT8 bmAttributes;  UINT16 wMaxPktSize;   UINT8 bInterval;
} USB_EP_DSC;

#endif //USBDEFS_STD_DSC_H

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
