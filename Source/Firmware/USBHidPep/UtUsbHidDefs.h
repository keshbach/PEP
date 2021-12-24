/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtUsbHidDefs_H)
#define	UtUsbHidDefs_H

/******************************************************************************
 * Class-Specific Requests
 * Device Class Definition for Human Interface Devices (HID)
 *     USB 1.11 Chapter 7.2
 *****************************************************************************/

/* Class-Specific Requests */
#define CUsbHidClassRequestGetReport   0x01
#define CUsbHidClassRequestGetIdle     0x02
#define CUsbHidClassRequestGetProtocol 0x03
#define CUsbHidClassRequestSetReport   0x09
#define CUsbHidClassRequestSetIdle     0x0A
#define CUsbHidClassRequestSetProtocol 0x0B

/* Class Descriptor Types */
#define CUsbHidClassDescriptorTypeHID                0x21
#define CUsbHidClassDescriptorTypeReport             0x22
#define CUsbHidClassDescriptorTypePhysicalDescriptor 0x23

typedef struct tagTUsbHidReportDescriptor
{
    UINT8 bDescriptorType;
    UINT16 wDescriptorLength;
} TUsbHidReportDescriptor;

typedef struct tagTUsbHidDescriptor
{
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT16 bcdHID;
    UINT8 bCountryCode;
    UINT8 bNumDescriptors;
    TUsbHidReportDescriptor ReportDescriptor[1];
} TUsbHidDescriptor;

#endif /* UtUsbHidDefs_H */

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
