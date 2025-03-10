#if !defined(_USB_CH9_H_)
#define _USB_CH9_H_

// *****************************************************************************
// *****************************************************************************
// Section: USB Descriptors
// *****************************************************************************
// *****************************************************************************

#define USB_DESCRIPTOR_DEVICE           0x01 // bDescriptorType for a Device Descriptor.
#define USB_DESCRIPTOR_CONFIGURATION    0x02 // bDescriptorType for a Configuration Descriptor.
#define USB_DESCRIPTOR_STRING           0x03 // bDescriptorType for a String Descriptor.
#define USB_DESCRIPTOR_INTERFACE        0x04 // bDescriptorType for an Interface Descriptor.
#define USB_DESCRIPTOR_ENDPOINT         0x05 // bDescriptorType for an Endpoint Descriptor.
//#define USB_DESCRIPTOR_DEVICE_QUALIFIER 0x06 // bDescriptorType for a Device Qualifier.
//#define USB_DESCRIPTOR_OTHER_SPEED      0x07 // bDescriptorType for a Other Speed Configuration.
//#define USB_DESCRIPTOR_INTERFACE_POWER  0x08 // bDescriptorType for Interface Power.
//#define USB_DESCRIPTOR_OTG              0x09 // bDescriptorType for an OTG Descriptor.

// *****************************************************************************
/* USB Device Descriptor Structure

This struct defines the structure of a USB Device Descriptor.  Note that this
structure may need to be packed, or even accessed as bytes, to properly access
the correct fields when used on some device architectures.
*/
typedef struct __attribute__ ((packed)) tagTUsbDeviceDescriptor
{
    UINT8  bLength;            // Length of this descriptor.
    UINT8  bDescriptorType;    // DEVICE descriptor type (USB_DESCRIPTOR_DEVICE).
    UINT16 bcdUSB;             // USB Spec Release Number (BCD).
    UINT8  bDeviceClass;       // Class code (assigned by the USB-IF). 0xFF-Vendor specific.
    UINT8  bDeviceSubClass;    // Subclass code (assigned by the USB-IF).
    UINT8  bDeviceProtocol;    // Protocol code (assigned by the USB-IF). 0xFF-Vendor specific.
    UINT8  bMaxPacketSize0;    // Maximum packet size for endpoint 0.
    UINT16 idVendor;           // Vendor ID (assigned by the USB-IF).
    UINT16 idProduct;          // Product ID (assigned by the manufacturer).
    UINT16 bcdDevice;          // Device release number (BCD).
    UINT8  iManufacturer;      // Index of String Descriptor describing the manufacturer.
    UINT8  iProduct;           // Index of String Descriptor describing the product.
    UINT8  iSerialNumber;      // Index of String Descriptor with the device's serial number.
    UINT8  bNumConfigurations; // Number of possible configurations.
} TUsbDeviceDescriptor;

// *****************************************************************************
/* USB Configuration Descriptor Structure

This struct defines the structure of a USB Configuration Descriptor.  Note that this
structure may need to be packed, or even accessed as bytes, to properly access
the correct fields when used on some device architectures.
*/
typedef struct __attribute__ ((packed)) tagTUsbConfigurationDescriptor
{
    UINT8  bLength;              // Length of this descriptor.
    UINT8  bDescriptorType;      // CONFIGURATION descriptor type (USB_DESCRIPTOR_CONFIGURATION).
    UINT16 wTotalLength;         // Total length of all descriptors for this configuration.
    UINT8  bNumInterfaces;       // Number of interfaces in this configuration.
    UINT8  bConfigurationValue;  // Value of this configuration (1 based).
    UINT8  iConfiguration;       // Index of String Descriptor describing the configuration.
    UINT8  bmAttributes;         // Configuration characteristics.
    UINT8  bMaxPower;            // Maximum power consumed by this configuration.
} TUsbConfigurationDescriptor;

// *****************************************************************************
/* USB Interface Descriptor Structure

This struct defines the structure of a USB Interface Descriptor.  Note that this
structure may need to be packed, or even accessed as bytes, to properly access
the correct fields when used on some device architectures.
*/
typedef struct __attribute__ ((packed)) TUsbInterfaceDescriptor
{
    UINT8 bLength;               // Length of this descriptor.
    UINT8 bDescriptorType;       // INTERFACE descriptor type (USB_DESCRIPTOR_INTERFACE).
    UINT8 bInterfaceNumber;      // Number of this interface (0 based).
    UINT8 bAlternateSetting;     // Value of this alternate interface setting.
    UINT8 bNumEndpoints;         // Number of endpoints in this interface.
    UINT8 bInterfaceClass;       // Class code (assigned by the USB-IF).  0xFF-Vendor specific.
    UINT8 bInterfaceSubClass;    // Subclass code (assigned by the USB-IF).
    UINT8 bInterfaceProtocol;    // Protocol code (assigned by the USB-IF).  0xFF-Vendor specific.
    UINT8 iInterface;            // Index of String Descriptor describing the interface.
} TUsbInterfaceDescriptor;

// *****************************************************************************
/* USB Endpoint Descriptor Structure

This struct defines the structure of a USB Endpoint Descriptor.  Note that this
structure may need to be packed, or even accessed as bytes, to properly access
the correct fields when used on some device architectures.
*/
typedef struct __attribute__ ((packed)) tagTUsbEndpointDescriptor
{
    UINT8  bLength;               // Length of this descriptor.
    UINT8  bDescriptorType;       // ENDPOINT descriptor type (USB_DESCRIPTOR_ENDPOINT).
    UINT8  bEndpointAddress;      // Endpoint address. Bit 7 indicates direction (0=OUT, 1=IN).
    UINT8  bmAttributes;          // Endpoint transfer type.
    UINT16 wMaxPacketSize;        // Maximum packet size.
    UINT8  bInterval;             // Polling interval in frames.
} TUsbEndpointDescriptor;

// ******************************************************************
// Section: USB String Descriptor Structure
// ******************************************************************
// This structure describes the USB string descriptor.  The string
// descriptor provides user-readable information about various aspects of
// the device.  The first string descriptor (string descriptor zero (0)),
// provides a list of the number of languages supported by the set of
// string descriptors for this device instead of an actual string.
//
// Note: The strings are in 2-byte-per-character unicode, not ASCII.
//
// Note: This structure only describes the "header" of the string
// descriptor.  The actual data (either the language ID array or the
// array of unicode characters making up the string, must be allocated
// immediately following this header with no padding between them.

typedef struct __attribute__ ((packed)) tagTUsbStringDescriptor
{
    UINT8 bLength;             // Size of this descriptor
    UINT8 bDescriptorType;     // Type, USB_DSC_STRING
} TUsbStringDescriptor;

// ******************************************************************
// Section: USB Device Qualifier Descriptor Structure
// ******************************************************************
// This structure describes the device qualifier descriptor.  The device
// qualifier descriptor provides overall device information if the device
// supports "other" speeds.
//
// Note: A high-speed device may support "other" speeds (ie. full or low).
// If so, it may need to implement the the device qualifier and other
// speed descriptors.

typedef struct __attribute__ ((packed)) tagTUsbDeviceQualifierDescriptor
{
    UINT8  bLength;              // Size of this descriptor
    UINT8  bType;                // Type, always TUsbDeviceQualifierDescriptor
    UINT16 bcdUSB;               // USB spec version, in BCD
    UINT8  bDeviceClass;         // Device class code
    UINT8  bDeviceSubClass;      // Device sub-class code
    UINT8  bDeviceProtocol;      // Device protocol
    UINT8  bMaxPacketSize0;      // EP0, max packet size
    UINT8  bNumConfigurations;   // Number of "other-speed" configurations
    UINT8  bReserved;            // Always zero (0)
} TUsbDeviceQualifierDescriptor;

// ******************************************************************
// Section: USB Setup Packet Structure
// ******************************************************************
// This structure describes the data contained in a USB standard device
// request's setup packet.  It is the data packet sent from the host to
// the device to control and configure the device.
//
// Note: Refer to the USB 2.0 specification for additional details on the
// usage of the setup packet and standard device requests.
typedef union __attribute__ ((packed)) tagTUsbSetupPacket
{
    struct __attribute__ ((packed))
    {
        UINT8  bmRequestType; //from table 9-2 of USB2.0 spec
        UINT8  bRequest;      //from table 9-2 of USB2.0 spec
        UINT16 wValue;        //from table 9-2 of USB2.0 spec
        UINT16 wIndex;        //from table 9-2 of USB2.0 spec
        UINT16 wLength;       //from table 9-2 of USB2.0 spec
    };
    struct __attribute__ ((packed))
    {
        unsigned :8;
        unsigned :8;
        UINT16 W_Value;  //from table 9-2 of USB2.0 spec, allows byte/bitwise access
        UINT16 W_Index;  //from table 9-2 of USB2.0 spec, allows byte/bitwise access
        UINT16 W_Length; //from table 9-2 of USB2.0 spec, allows byte/bitwise access
    };
    struct __attribute__ ((packed))
    {
        unsigned Recipient:5;   //Device, Interface, Endpoint, Other
        unsigned RequestType:2; //Standard, Class, Vendor, Reserved
        unsigned DataDir:1;     //Host-to-device, Device-to-host
        unsigned :8;
        UINT8 bFeature;         //DEVICE_REMOTE_WAKEUP, ENDPOINT_HALT
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct __attribute__ ((packed))
    {
        union                           // offset   description
        {                               // ------   ------------------------
            UINT8 bmRequestType;        //   0      Bit-map of request type
            struct
            {
                UINT8   recipient:  5;  //          Recipient of the request
                UINT8   type:       2;  //          Type of request
                UINT8   direction:  1;  //          Direction of data X-fer
            };
        } requestInfo;
    };
    struct __attribute__ ((packed))
    {
        unsigned :8;
        unsigned :8;
        UINT8 bDscIndex;       // For Configuration and String DSC Only
        UINT8 bDescriptorType; // Device, Configuration, String
        UINT16 wLangID;        // Language ID
        unsigned :8;
        unsigned :8;
    };
    struct __attribute__ ((packed))
    {
        unsigned :8;
        unsigned :8;
        UINT8 bDevADR;  // Device Address 0-127
        UINT8 bDevADRH; // Must equal zero
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct __attribute__ ((packed))
    {
        unsigned :8;
        unsigned :8;
        UINT8 bConfigurationValue; // Configuration Value 0-255
        UINT8 bCfgRSD;             // Must equal zero (Reserved)
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
    struct __attribute__ ((packed))
    {
        unsigned :8;
        unsigned :8;
        UINT8 bAltID;    // Alternate Setting Value 0-255
        UINT8 bAltID_H;  // Must equal zero
        UINT8 bIntfID;   // Interface Number Value 0-255
        UINT8 bIntfID_H; // Must equal zero
        unsigned :8;
        unsigned :8;
    };
    struct __attribute__ ((packed))
    {
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        UINT8 bEPID;   // Endpoint ID (Number & Direction)
        UINT8 bEPID_H; // Must equal zero
        unsigned :8;
        unsigned :8;
    };
    struct __attribute__ ((packed))
    {
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned :8;
        unsigned EPNum:4; // Endpoint Number 0-15
        unsigned :3;
        unsigned EPDir:1; // Endpoint Direction: 0-OUT, 1-IN
        unsigned :8;
        unsigned :8;
        unsigned :8;
    };
} TUsbSetupPacket;

// ******************************************************************
// ******************************************************************
// Section: USB Specification Constants
// ******************************************************************
// ******************************************************************

// Section: Valid PID Values
#define PID_OUT                                 0x1     // PID for an OUT token
#define PID_ACK                                 0x2     // PID for an ACK handshake
#define PID_DATA0                               0x3     // PID for DATA0 data
#define PID_PING                                0x4     // Special PID PING
#define PID_SOF                                 0x5     // PID for a SOF token
#define PID_NYET                                0x6     // PID for a NYET handshake
#define PID_DATA2                               0x7     // PID for DATA2 data
#define PID_SPLIT                               0x8     // Special PID SPLIT
#define PID_IN                                  0x9     // PID for a IN token
#define PID_NAK                                 0xA     // PID for a NAK handshake
#define PID_DATA1                               0xB     // PID for DATA1 data
#define PID_PRE                                 0xC     // Special PID PRE (Same as PID_ERR)
#define PID_ERR                                 0xC     // Special PID ERR (Same as PID_PRE)
#define PID_SETUP                               0xD     // PID for a SETUP token
#define PID_STALL                               0xE     // PID for a STALL handshake
#define PID_MDATA                               0xF     // PID for MDATA data

#define PID_MASK_DATA                           0x03    // Data PID mask
#define PID_MASK_DATA_SHIFTED                  (PID_MASK_DATA << 2) // Data PID shift to proper position

// Section: Standard Device Requests

#define USB_REQUEST_GET_STATUS                  0       // Standard Device Request - GET STATUS
#define USB_REQUEST_CLEAR_FEATURE               1       // Standard Device Request - CLEAR FEATURE
#define USB_REQUEST_SET_FEATURE                 3       // Standard Device Request - SET FEATURE
#define USB_REQUEST_SET_ADDRESS                 5       // Standard Device Request - SET ADDRESS
#define USB_REQUEST_GET_DESCRIPTOR              6       // Standard Device Request - GET DESCRIPTOR
#define USB_REQUEST_SET_DESCRIPTOR              7       // Standard Device Request - SET DESCRIPTOR
#define USB_REQUEST_GET_CONFIGURATION           8       // Standard Device Request - GET CONFIGURATION
#define USB_REQUEST_SET_CONFIGURATION           9       // Standard Device Request - SET CONFIGURATION
#define USB_REQUEST_GET_INTERFACE               10      // Standard Device Request - GET INTERFACE
#define USB_REQUEST_SET_INTERFACE               11      // Standard Device Request - SET INTERFACE
#define USB_REQUEST_SYNCH_FRAME                 12      // Standard Device Request - SYNCH FRAME

#define USB_FEATURE_ENDPOINT_HALT               0       // CLEAR/SET FEATURE - Endpoint Halt
#define USB_FEATURE_DEVICE_REMOTE_WAKEUP        1       // CLEAR/SET FEATURE - Device remote wake-up
#define USB_FEATURE_TEST_MODE                   2       // CLEAR/SET FEATURE - Test mode

// Section: Setup Data Constants

#define USB_SETUP_HOST_TO_DEVICE                0x00    // Device Request bmRequestType transfer direction - host to device transfer
#define USB_SETUP_DEVICE_TO_HOST                0x80    // Device Request bmRequestType transfer direction - device to host transfer
#define USB_SETUP_TYPE_STANDARD                 0x00    // Device Request bmRequestType type - standard
#define USB_SETUP_TYPE_CLASS                    0x20    // Device Request bmRequestType type - class
#define USB_SETUP_TYPE_VENDOR                   0x40    // Device Request bmRequestType type - vendor
#define USB_SETUP_RECIPIENT_DEVICE              0x00    // Device Request bmRequestType recipient - device
#define USB_SETUP_RECIPIENT_INTERFACE           0x01    // Device Request bmRequestType recipient - interface
#define USB_SETUP_RECIPIENT_ENDPOINT            0x02    // Device Request bmRequestType recipient - endpoint
#define USB_SETUP_RECIPIENT_OTHER               0x03    // Device Request bmRequestType recipient - other

#define USB_SETUP_HOST_TO_DEVICE_BITFIELD       (USB_SETUP_HOST_TO_DEVICE >> 7) // Device Request bmRequestType transfer direction - host to device transfer - bit definition
#define USB_SETUP_DEVICE_TO_HOST_BITFIELD       (USB_SETUP_DEVICE_TO_HOST >> 7) // Device Request bmRequestType transfer direction - device to host transfer - bit definition
#define USB_SETUP_TYPE_STANDARD_BITFIELD        (USB_SETUP_TYPE_STANDARD >> 5)  // Device Request bmRequestType type - standard
#define USB_SETUP_TYPE_CLASS_BITFIELD           (USB_SETUP_TYPE_CLASS >> 5)     // Device Request bmRequestType type - class
#define USB_SETUP_TYPE_VENDOR_BITFIELD          (USB_SETUP_TYPE_VENDOR >> 5)    // Device Request bmRequestType type - vendor
#define USB_SETUP_RECIPIENT_DEVICE_BITFIELD     (USB_SETUP_RECIPIENT_DEVICE)    // Device Request bmRequestType recipient - device
#define USB_SETUP_RECIPIENT_INTERFACE_BITFIELD  (USB_SETUP_RECIPIENT_INTERFACE) // Device Request bmRequestType recipient - interface
#define USB_SETUP_RECIPIENT_ENDPOINT_BITFIELD   (USB_SETUP_RECIPIENT_ENDPOINT)  // Device Request bmRequestType recipient - endpoint
#define USB_SETUP_RECIPIENT_OTHER_BITFIELD      (USB_SETUP_RECIPIENT_OTHER)     // Device Request bmRequestType recipient - other

/* Configuration Attributes */
#define _DEFAULT    (0x01 << 7)     // Default Value (Bit 7 is set)
#define _HNP	    (0x01 << 1)     // HNP (Supports if set)
#define _SRP	  	(0x01)		    // SRP (Supports if set)

/* Endpoint Transfer Type */
#define CEndpointTransferTypeBulk 0x02 // Bulk Transfer

// These are the directional indicators used for the USBTransferOnePacket()
// function.
#define OUT_FROM_HOST 0
#define IN_TO_HOST 1

#endif  // _USB_CH9_H_
