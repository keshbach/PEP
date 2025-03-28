/*********************************************************************
 * Descriptor specific type definitions are defined in: usbd.h
 ********************************************************************/

#if !defined(USB_CONFIG_H)
#define USB_CONFIG_H

/** DEFINITIONS ****************************************************/
#define USB_EP0_BUFF_SIZE 64 // Valid Options: 8, 16, 32, or 64 bytes.
                             // Using larger options take more SRAM, but
                             // does not provide much advantage in most types
                             // of applications.  Exceptions to this, are applications
                             // that use EP0 IN or OUT for sending large amounts of
                             // application related data.
									
#define USB_MAX_NUM_INT   1   // For tracking Alternate Setting
#define USB_MAX_EP_NUMBER 1

//Make sure only one of the below "#define USB_PING_PONG_MODE"
//is uncommented.
//#define USB_PING_PONG_MODE USB_PING_PONG__NO_PING_PONG
#define USB_PING_PONG_MODE USB_PING_PONG__FULL_PING_PONG
//#define USB_PING_PONG_MODE USB_PING_PONG__EP0_OUT_ONLY
//#define USB_PING_PONG_MODE USB_PING_PONG__ALL_BUT_EP0		//NOTE: This mode is not supported in PIC18F4550 family rev A3 devices


#define USB_POLLING
//#define USB_INTERRUPT

/* Parameter definitions are defined in usb_device.h */
#define USB_PULLUP_OPTION USB_PULLUP_ENABLE
//#define USB_PULLUP_OPTION USB_PULLUP_DISABLED

#define USB_TRANSCEIVER_OPTION USB_INTERNAL_TRANSCEIVER
//External Transceiver support is not available on all product families.  Please
//  refer to the product family datasheet for more information if this feature
//  is available on the target processor.
//#define USB_TRANSCEIVER_OPTION USB_EXTERNAL_TRANSCEIVER

#define USB_SPEED_OPTION USB_FULL_SPEED

//------------------------------------------------------------------------------------------------------------------
//Option to enable auto-arming of the status stage of control transfers, if no
//"progress" has been made for the USB_STATUS_STAGE_TIMEOUT value.
//If progress is made (any successful transactions completing on EP0 IN or OUT)
//the timeout counter gets reset to the USB_STATUS_STAGE_TIMEOUT value.
//
//During normal control transfer processing, the USB stack or the application 
//firmware will call USBCtrlEPAllowStatusStage() as soon as the firmware is finished
//processing the control transfer.  Therefore, the status stage completes as 
//quickly as is physically possible.  The USB_ENABLE_STATUS_STAGE_TIMEOUTS 
//feature, and the USB_STATUS_STAGE_TIMEOUT value are only relevant, when:
//1.  The application uses the USBDeferStatusStage() API function, but never calls
//      USBCtrlEPAllowStatusStage().  Or:
//2.  The application uses host to device (OUT) control transfers with data stage,
//      and some abnormal error occurs, where the host might try to abort the control
//      transfer, before it has sent all of the data it claimed it was going to send.
//
//If the application firmware never uses the USBDeferStatusStage() API function,
//and it never uses host to device control transfers with data stage, then
//it is not required to enable the USB_ENABLE_STATUS_STAGE_TIMEOUTS feature.

#define USB_ENABLE_STATUS_STAGE_TIMEOUTS    //Comment this out to disable this feature.  

//Section 9.2.6 of the USB 2.0 specifications indicate that:
//1.  Control transfers with no data stage: Status stage must complete within 
//      50ms of the start of the control transfer.
//2.  Control transfers with (IN) data stage: Status stage must complete within 
//      50ms of sending the last IN data packet in fullfilment of the data stage.
//3.  Control transfers with (OUT) data stage: No specific status stage timing
//      requirement.  However, the total time of the entire control transfer (ex:
//      including the OUT data stage and IN status stage) must not exceed 5 seconds.
//
//Therefore, if the USB_ENABLE_STATUS_STAGE_TIMEOUTS feature is used, it is suggested
//to set the USB_STATUS_STAGE_TIMEOUT value to timeout in less than 50ms.  If the
//USB_ENABLE_STATUS_STAGE_TIMEOUTS feature is not enabled, then the USB_STATUS_STAGE_TIMEOUT
//parameter is not relevant.

#define USB_STATUS_STAGE_TIMEOUT     (UINT8)45 //Approximate timeout in milliseconds, except when
                                               //USB_POLLING mode is used, and USBDeviceTasks() is called at < 1kHz
                                               //In this special case, the timeout becomes approximately:
//Timeout(in milliseconds) = ((1000 * (USB_STATUS_STAGE_TIMEOUT - 1)) / (USBDeviceTasks() polling frequency in Hz))
//------------------------------------------------------------------------------------------------------------------


//When implemented, the Microsoft OS Descriptor allows the WinUSB driver package 
//installation to be automatic on Windows 8, and is therefore recommended.
#define IMPLEMENT_MICROSOFT_OS_DESCRIPTOR

//Some definitions, only needed when using the MS OS descriptor.
#if defined(IMPLEMENT_MICROSOFT_OS_DESCRIPTOR)
#define MICROSOFT_OS_DESCRIPTOR_INDEX   (UINT8)0xEE // Magic string index number for the Microsoft OS descriptor
#define GET_MS_DESCRIPTOR               (UINT8)0xEE // (arbitarily assigned, but should not clobber/overlap normal bRequests)
#define EXTENDED_COMPAT_ID              (UINT16)0x0004
#define EXTENDED_PROPERTIES             (UINT16)0x0005

typedef struct __attribute__ ((packed)) _MS_OS_DESCRIPTOR 
{
    UINT8 bLength;
    UINT8 bDscType;
    UINT16 string[7];
    UINT8 vendorCode;
    UINT8 bPad;
} MS_OS_DESCRIPTOR;

typedef struct __attribute__ ((packed)) _MS_COMPAT_ID_FEATURE_DESC
{
    UINT32 dwLength;
    UINT16 bcdVersion;
    UINT16 wIndex;
    UINT8 bCount;
    UINT8 Reserved[7];
    UINT8 bFirstInterfaceNumber;
    UINT8 Reserved1;
    UINT8 compatID[8];
    UINT8 subCompatID[8];
    UINT8 Reserved2[6];
} MS_COMPAT_ID_FEATURE_DESC;

typedef struct __attribute__ ((packed)) _MS_EXT_PROPERTY_FEATURE_DESC
{
    UINT32 dwLength;
    UINT16 bcdVersion;
    UINT16 wIndex;
    UINT16 wCount;
    UINT32 dwSize;
    UINT32 dwPropertyDataType;
    UINT16 wPropertyNameLength;
    UINT16 bPropertyName[20];
    UINT32 dwPropertyDataLength;
    UINT16 bPropertyData[39];
} MS_EXT_PROPERTY_FEATURE_DESC;

extern ROM MS_OS_DESCRIPTOR MSOSDescriptor;
extern ROM MS_COMPAT_ID_FEATURE_DESC CompatIDFeatureDescriptor;
extern ROM MS_EXT_PROPERTY_FEATURE_DESC ExtPropertyFeatureDescriptor;
#endif

#define USB_NUM_STRING_DESCRIPTORS 3

#endif // USB_CONFIG_H
