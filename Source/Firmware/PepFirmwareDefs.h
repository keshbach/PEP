/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepFirmwareDefs_H)
#define PepFirmwareDefs_H

#define CMicrochipFirmwareVendorID 0x04d8

#define CMicrochipHIDFirmwareProductID 0x003c
#define CMicrochipWinUSBFirmwareProductID 0x0052

// From the DeviceInterfaceGUID field of the mchpwinusb.inf file

//#define CMicrochipFirmwareDeviceInterfaceGuid L"58D07210-27C1-11DD-BD0B-0800200C9A66"
#define CPepFirmwareDeviceInterfaceGuid '5', '8', 'D', '0', '7', '2', '1', '0', '-', '2', '7', 'C', '1', '-', '1', '1', 'D', 'D', '-', 'B', 'D', '0', 'B', '-', '0', '8', '0', '0', '2', '0', '0', 'C', '9', 'A', '6', '6'

// my generated guid
//#define CPepFirmwareDeviceInterfaceGuid '1', 'C', 'D', 'E', '9', '5', 'B', 'C', '-', '7', 'F', '0', 'A', '-', '4', 'F', '2', '3', '-', '8', 'B', 'E', 'F', '-', 'C', '1', 'C', '3', 'F', 'D', '4', '9', 'B', 'D', 'A', 'D'

#define CPepFirmwareEndPointNumber 1

#define CPepFirmwareInEndPoint (CPepFirmwareEndPointNumber | 0x80)
#define CPepFirmwareOutEndPoint CPepFirmwareEndPointNumber

/* Commands */

#define CPepVersionCommand 0x01
#define CPepResetCommand 0x02
#define CPepSetProgrammerModeCommand 0x03
#define CPepSetVccModeCommand 0x04
#define CPepSetPinPulseModeCommand 0x05
#define CPepSetVppModeCommand 0x06
#define CPepReadDataCommand 0x07
#define CPepReadUserDataCommand 0x08
#define CPepReadUserDataWithDelayCommand 0x09
#define CPepProgramDataCommand 0x0A
#define CPepProgramUserDataCommand 0x0B
#define CPepSetDelaysCommand 0x0C
#define CPepDebugWritePortDataCommand 0x81

/* Error Codes */

#define CPepErrorSuccess 0x00
#define CPepErrorInvalidCommand 0x01
#define CPepErrorFailed 0x02
#define CPepErrorInvalidLength 0x03
#define CPepErrorInitializeData 0xFF

#if defined(_MSC_VER)
#if defined(_X86_)
#define MPackedAttribute
#pragma pack(push, 1)
#elif defined(_AMD64_)
#define MPackedAttribute
#pragma pack(push, 1)
#else
#error Need to specify cpu architecture to configure structure padding
#endif
#elif defined(__32MX250F128B__) || defined(__32MX440F256H__)
#define MPackedAttribute __attribute__ ((packed))
#else
#error Need to specify how to enable byte aligned structure padding
#endif

// Buffer for command to send the response must be 64 bytes.
//
// All data is little endian

typedef struct MPackedAttribute tagTUtReadUserData
{
    UINT32 nAddress;
    UINT8 nEnableOutputEnable;
    UINT8 nPerformRead;
} TUtReadUserData;

typedef struct MPackedAttribute tagTUtReadUserDataWithDelay
{
    UINT32 nAddress;
    UINT32 nDelayNanoSeconds; // Delay execute after set address and before performing a read
    UINT8 nPerformRead;
} TUtReadUserDataWithDelay;

typedef struct MPackedAttribute tagTUtProgramUserData
{
    UINT32 nAddress;
    UINT8 nPerformProgram;
    UINT8 nData;
} TUtProgramUserData;

typedef struct MPackedAttribute tagTUtPepCommandData
{
    UINT8 Command;
            
    union
    {
        UINT32 nProgrammerMode;
        UINT32 nVccMode;
        UINT32 nPinPulseMode;
        UINT32 nVppMode;
 
        struct tagReadData
        {
            UINT32 nAddress;
            UINT8 nDataLen; // 0 - 62
        } ReadData;

        struct tagReadUserData
        {
            TUtReadUserData Data[10];
            UINT8 nDataLen; // 0 - 10
        } ReadUserData;

        struct tagReadUserDataWithDelay
        {
            TUtReadUserDataWithDelay Data[6];
            UINT8 nDataLen; // 0 - 6
        } ReadUserDataWithDelay;

        struct tagProgramData
        {
            UINT32 nAddress;
            UINT8 nData[58];
            UINT8 nDataLen; // 0 - 57
        } ProgramData;

        struct tagProgramUserData
        {
            TUtProgramUserData Data[10];
            UINT8 nDataLen; // 0 - 10
        } ProgramUserData;

        struct tagDelays
        {
            UINT32 nChipEnableNanoSeconds;
            UINT32 nOutputEnableNanoSeconds;
        } Delays;

        struct tagTDebugWritePortData
        {
            UINT8 nWritePortData;
            UINT8 Padding[62];
        } DebugWritePortData;

        UINT8 Padding[63];
    } Data;
} TUtPepCommandData;

typedef struct MPackedAttribute tagTUtPepResponseData
{
    UINT8 PacketID;
    UINT8 ErrorCode;
    
    union
    {
        UINT16 Version[31];
        UINT8 nData[62];
        UINT8 Padding[62];
    } Data;
} TUtPepResponseData;

#if defined(_MSC_VER)
#pragma pack(pop)
#elif defined(__32MX250F128B__) || defined(__32MX440F256H__)
#else
#error Need to specify how to restore original structure padding
#endif

#endif // PepFirmwareDefs_H

/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/
