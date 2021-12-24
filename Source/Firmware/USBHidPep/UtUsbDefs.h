/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtUsbDefs_H)
#define	UtUsbDefs_H

/******************************************************************************
 * Standard Request Codes
 * USB 2.0 Spec Ref Table 9-4
 *****************************************************************************/

#define CUsbRequestCodeGetStatus        0
#define CUsbRequestCodeClearFeature     1
#define CUsbRequestCodeSetFeature       3
#define CUsbRequestCodeSetAddress       5
#define CUsbRequestCodeGetDescriptor    6
#define CUsbRequestCodeSetDescriptor    7
#define CUsbRequestCodeGetConfiguration 8
#define CUsbRequestCodeSetConfiguration 9
#define CUsbRequestCodeGetInterface     10
#define CUsbRequestCodeSetInterface     11
#define CUsbRequestCodeSynchFrame       12

/******************************************************************************
 * Descriptor Types
 * USB 2.0 Spec Ref Table 9-5
 *****************************************************************************/

#define CUsbDescriptorTypeDevice        1
#define CUsbDescriptorTypeConfiguration 2
#define CUsbDescriptorTypeString        3
#define CUsbDescriptorTypeInterface     4
#define CUsbDescriptorTypeEndpoint      5

/******************************************************************************
 * Standard Feature Selectors
 * USB 2.0 Spec Ref Table 9-6
 *****************************************************************************/

#define CUsbFeatureSelectorDeviceRemoteWakeup 1
#define CUsbFeatureSelectorEndpointHalt       0

typedef struct tagTUsbDescriptorHeader
{
    UINT8 bLength;
    UINT8 bDescriptorType;
} TUsbDescriptorHeader;

/******************************************************************************
 * USB Device Request Structure
 * USB 2.0 Spec Ref Table 9-2
 *****************************************************************************/

typedef struct tagTUsbDeviceRequest
{
    unsigned Recipient:5;       // Device, Interface, Endpoint, Other, Reserved
    unsigned RequestType:2;     // Standard, Class, Vendor, Reserved
    unsigned DataTransferDir:1; // Host-to-device, Device-to-host
    UINT8 bRequest;    
    UINT16 wValue;
    UINT16 wIndex;
    UINT16 wLength;
} TUsbDeviceRequest;

/******************************************************************************
 * USB Device Descriptor Structure
 * USB 2.0 Spec Ref Table 9-7
 *****************************************************************************/

typedef struct tagTUsbDeviceDescriptor
{
    UINT8  bLength;
    UINT8  bDescriptorType;
    UINT16 bcdUSB;
    UINT8  bDeviceClass;
    UINT8  bDeviceSubClass;
    UINT8  bDeviceProtocol;
    UINT8  bMaxPacketSize0;
    UINT16 idVendor;
    UINT16 idProduct;
    UINT16 bcdDevice;
    UINT8  iManufacturer;
    UINT8  iProduct;
    UINT8  iSerialNumber;
    UINT8  bNumConfigurations;
} TUsbDeviceDescriptor;

/******************************************************************************
 * USB Standard Configuration Descriptor Structure
 * USB 2.0 Spec Ref Table 9-8
 *****************************************************************************/

typedef struct tagTUsbConfigurationDescriptor
{
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT16 wTotalLength;
    UINT8 bNumInterfaces;
    UINT8 bConfigurationValue;
    UINT8 iConfiguration;
    unsigned :5;              // Reserved (reset to zero)
    unsigned RemoteWakeup:1;
    unsigned SelfPowered:1; 
    unsigned :1;              // Reserved (set to one)
    UINT8 bMaxPower;
} TUsbConfigurationDescriptor;

/******************************************************************************
 * USB Standard Interface Descriptor Structure
 * USB 2.0 Spec Ref Table 9-9
 *****************************************************************************/

typedef struct tagTUsbInterfaceDescriptor
{
    UINT8 bLength;
    UINT8 bDescriptorType;
    UINT8 bInterfaceNumber;
    UINT8 bAlternateSetting;
    UINT8 bNumEndpoints;
    UINT8 bInterfaceClass;
    UINT8 bInterfaceSubClass;
    UINT8 bInterfaceProtocol;
    UINT8 iInterface;
} TUsbInterfaceDescriptor;

/******************************************************************************
 * USB Standard Endpoint Descriptor Structure
 * USB 2.0 Spec Ref Table 9-10
 *****************************************************************************/

typedef struct tagTUsbEndpointDescriptor
{
    UINT8 bLength;
    UINT8 bDescriptorType;
    unsigned EndpointNumber:4;
    unsigned :3;              // Reserved (reset to zero)
    unsigned Direction:1;     // Direction (0 - OUT endpoint, 1 - IN endpoint)
    unsigned TransferType:2;  // 00 - Control, 01 - Isochronous, 10 - Bulk, 11 - Interrupt)
    unsigned :6;              // Reserved
    UINT16 wMaxPktSize;
    UINT8 bInterval;
} TUsbEndpointDescriptor;

#endif /* UtUsbDefs_H */

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
