#include "UtPortTypeDefs.h"

#include "usb.h"
#include "usb_function_generic.h"

#include <Firmware/PepFirmwareDefs.h>

typedef struct __attribute__ ((packed)) tagTDescriptorData {
    TUsbConfigurationDescriptor ConfigurationDescriptor;
    TUsbInterfaceDescriptor InterfaceDescriptor;
    TUsbEndpointDescriptor OutEndpointDescriptor;
    TUsbEndpointDescriptor InEndpointDescriptor;
} TDescriptorData;

typedef struct __attribute__ ((packed)) tagTUsbStringDescriptorCodePage {
    TUsbStringDescriptor UsbStringDescriptor;
    UINT16 CodePage;
} TUsbStringDescriptorCodePage;

/* Device Descriptor */
ROM TUsbDeviceDescriptor g_DeviceDescriptor = {
    sizeof(g_DeviceDescriptor),             // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,                  // DEVICE descriptor type
    0x0200,                                 // USB Spec Release Number in BCD format
    0x00,                                   // Class Code
    0x00,                                   // Subclass code
    0x00,                                   // Protocol code
    USB_EP0_BUFF_SIZE,                      // Max packet size for EP0, see usb_config.h
    CPidCodesGithubFirmwareVendorID,        // Vendor ID: pidcodes.github.com
    CPidCodesGithubWinUSBFirmwareProductID, // Product ID: pidcodes.github.com WinUSB
    0x0000,                                 // Device release number in BCD format
    0x01,                                   // Manufacturer string index
    0x02,                                   // Product string index
    0x00,                                   // Device serial number string index
    0x01                                    // Number of possible configurations
};

static ROM TDescriptorData l_ConfigDescriptor = {
    /* Configuration Descriptor */
    {
    sizeof(l_ConfigDescriptor.ConfigurationDescriptor),
    USB_DESCRIPTOR_CONFIGURATION, // CONFIGURATION descriptor
    0x0020,                       // Total length of data for this cfg
    1,                            // Number of interfaces in this cfg
    1,                            // Index value of this configuration
    0,                            // Configuration string index
    _DEFAULT,                     // Attributes, see usb_device.h
    50                            // Max power consumption (2X mA)
    },

    /* Interface Descriptor */
    {
    sizeof(l_ConfigDescriptor.InterfaceDescriptor),
    USB_DESCRIPTOR_INTERFACE, // INTERFACE descriptor type
    0,                        // Interface Number
    0,                        // Alternate Setting Number
    2,                        // Number of endpoints in this interface
    0xFF,                     // Class code
    0xFF,                     // Subclass code
    0xFF,                     // Protocol code
    0                         // Interface string index
    },

    /* Endpoint Descriptor */
    {
    sizeof(l_ConfigDescriptor.OutEndpointDescriptor),
    USB_DESCRIPTOR_ENDPOINT,   // Endpoint Descriptor
    CPepFirmwareOutEndPoint,   // EndpointAddress
    CEndpointTransferTypeBulk, // Attributes
    64,                        // size
    0x01,                      // Interval
    },
    
    /* Endpoint Descriptor */
    {
    sizeof(l_ConfigDescriptor.InEndpointDescriptor),
    USB_DESCRIPTOR_ENDPOINT,   // Endpoint Descriptor
    CPepFirmwareInEndPoint,    // EndpointAddress
    CEndpointTransferTypeBulk, // Attributes
    64,                        // size
    0x01                       // Interval
    }
};

// Language code string descriptor
static ROM TUsbStringDescriptorCodePage l_StringDescriptorCodePage = {
    {
    sizeof(l_StringDescriptorCodePage),
    USB_DESCRIPTOR_STRING
    },
    0x0409 // English (USA) Locale Identifier
};

// Manufacturer string descriptor
static ROM struct __attribute__ ((packed)) {
    TUsbStringDescriptor UsbStringDescriptor;
    UINT16 string[13];
}
l_StringDescriptorManufacturer = {
    {
    sizeof(l_StringDescriptorManufacturer),
    USB_DESCRIPTOR_STRING
    },
    {'K', 'e', 'v', 'i', 'n', ' ', 'E', 's', 'h', 'b', 'a', 'c', 'h'} 
};

// Product string descriptor

static ROM struct __attribute__ ((packed)) {
    TUsbStringDescriptor UsbStringDescriptor;
    UINT16 string[15];
}
l_StringDescriptorProduct = {
    {
    sizeof(l_StringDescriptorProduct),
    USB_DESCRIPTOR_STRING
    },
    {'U', 'S', 'B', ' ', 'T', 'o', ' ', 'E', 'P', 'R', 'O', 'M', '+', ' ', '2'}
};

// Array of configuration descriptors
ROM UINT8* ROM USB_CD_Ptr[] = {
    (ROM UINT8* ROM)&l_ConfigDescriptor
};

// Array of string descriptors
ROM UINT8* ROM USB_SD_Ptr[USB_NUM_STRING_DESCRIPTORS] = {
    (ROM UINT8* ROM)&l_StringDescriptorCodePage,
    (ROM UINT8* ROM)&l_StringDescriptorManufacturer,
    (ROM UINT8* ROM)&l_StringDescriptorProduct
};

#if defined(IMPLEMENT_MICROSOFT_OS_DESCRIPTOR)
// Microsoft "OS Descriptor" - This descriptor is based on a Microsoft specific 
// specification (not part of the standard USB 2.0 specs or class specs). 
// Implementing this special descriptor allows WinUSB driver package installation
// to be automatic on Windows 8.  For additional details, see:
// http://msdn.microsoft.com/en-us/library/windows/hardware/hh450799(v=vs.85).aspx
ROM MS_OS_DESCRIPTOR MSOSDescriptor =
{   
    sizeof(MSOSDescriptor),              // bLength - length of this descriptor in bytes
    USB_DESCRIPTOR_STRING,               // bDescriptorType - "string"
    {'M', 'S', 'F', 'T', '1', '0', '0'}, // qwSignature - special values that specifies the OS descriptor spec version that this firmware implements
    GET_MS_DESCRIPTOR,                   // bMS_VendorCode - defines the "GET_MS_DESCRIPTOR" bRequest literal value
    0x00                                 // bPad - always 0x00
};
    
// Extended Compat ID OS Feature Descriptor
ROM MS_COMPAT_ID_FEATURE_DESC CompatIDFeatureDescriptor =
{
    //----------Header Section--------------
    sizeof(CompatIDFeatureDescriptor),    // dwLength
    0x0100,                               // bcdVersion = 1.00
    EXTENDED_COMPAT_ID,                   // wIndex
    0x01,                                 // bCount - 0x01 "Function Section(s)" implemented in this descriptor
    {0, 0, 0, 0, 0, 0, 0},                // Reserved[7]
    //----------Function Section 1----------
    0x00,                                 // bFirstInterfaceNumber: the WinUSB interface in this firmware is interface #0
    0x01,                                 // Reserved - fill this reserved byte with 0x01 according to documentation
    {'W', 'I', 'N', 'U', 'S', 'B', 0, 0}, // compatID - "WINUSB" (with two null terminators to fill all 8 bytes)
    {0, 0, 0, 0, 0, 0, 0, 0},             // subCompatID - eight bytes of 0
    {0, 0, 0, 0, 0, 0}                    // Reserved
};    
    
// Extended Properties OS Feature Descriptor
ROM MS_EXT_PROPERTY_FEATURE_DESC ExtPropertyFeatureDescriptor =
{
    //----------Header Section--------------
    sizeof(ExtPropertyFeatureDescriptor),   // dwLength
    0x0100,                                 // bcdVersion = 1.00
    EXTENDED_PROPERTIES,                    // wIndex
    0x0001,                                 // wCount - 0x0001 "Property Sections" implemented in this descriptor
    //----------Property Section 1----------
    132,                                    // dwSize - 132 bytes in this Property Section
    0x00000001,                             // dwPropertyDataType (Unicode string)
    40,                                     // wPropertyNameLength - 40 bytes in the bPropertyName field
    {'D', 'e', 'v', 'i', 'c', 'e', 'I', 'n', 't', 'e', 'r', 'f', 'a', 'c', 'e', 'G', 'U', 'I', 'D', 0x0000}, // bPropertyName - "DeviceInterfaceGUID"
    78,                                     // dwPropertyDataLength - 78 bytes in the bPropertyData field (GUID value in UNICODE formatted string, with braces and dashes)
    {'{', CPepFirmwareDeviceInterfaceGuid, '}', 0}
};    
#endif
