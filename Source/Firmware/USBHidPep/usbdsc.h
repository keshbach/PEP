/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

/*********************************************************************
 * Descriptor specific type definitions are defined in:
 * system\usb\usbdefs\usbdefs_std_dsc.h
 ********************************************************************/

#if !defined(USBDSC_H)
#define USBDSC_H

#include "UtTypeDefs.h"
#include "usbcfg.h"

#include "hid.h"

#include "usb.h"

typedef struct tagTConfiguration
{
    USB_CFG_DSC cd01;
    USB_INTF_DSC i00a00;
    USB_HID_DSC hid_i00a00;
    USB_EP_DSC ep01i_i00a00;
    USB_EP_DSC ep01o_i00a00;
} TConfiguration;

extern rom USB_DEV_DSC g_DeviceDescriptor;
extern rom TConfiguration g_Configuration1;
extern rom const UINT8 *rom g_USB_CD_Ptr[];
extern rom const UINT8 *rom g_USB_SD_Ptr[];

extern rom struct { UINT8 report[HID_RPT01_SIZE]; } g_HidReport1;
extern rom TFunc g_ClassReqHandler[1];

#endif //USBDSC_H

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
