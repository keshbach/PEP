/***************************************************************************/
/*  Copyright (C) 2006-2022 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepFirmwareDefs_H)
#define PepFirmwareDefs_H

#define CPepFirmwareVendorID 0x04d8
#define CPepFirmwareProductID 0x003c

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
#define CPepSetDelaysCommand 0x0E

/* Error Codes */

#define CPepErrorSuccess 0x00
#define CPepErrorInvalidCommand 0x01
#define CPepErrorFailed 0x02
#define CPepErrorInvalidLength 0x03

#if defined(_MSC_VER)
#if defined(_X86_)
#pragma pack(push, 1)
#elif defined(_AMD64_)
#pragma pack(push, 1)
#else
#error Need to specify cpu architecture to configure structure padding
#endif
#elif defined(__XC8) || defined(__18CXX)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

// Buffer for command to send the response must be 64 bytes.
//
// All data is little endian

typedef struct tagTUtReadUserData
{
    UINT32 nAddress;
    UINT8 nEnableOutputEnable;
    UINT8 nPerformRead;
} TUtReadUserData;

typedef struct tagTUtReadUserDataWithDelay
{
    UINT32 nAddress;
    UINT32 nDelayNanoSeconds; // Delay execute after set address and before performing a read
    UINT8 nPerformRead;
} TUtReadUserDataWithDelay;

typedef struct tagTUtProgramUserData
{
    UINT32 nAddress;
    UINT8 nPerformProgram;
    UINT8 nData;
} TUtProgramUserData;

typedef struct tagTUtPepCommandData
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
            UINT8 nDataLen; // 0 - 63
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
            UINT8 nDataLen; // 0 - 58
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

        UINT8 Padding[63];
    } Data;
} TUtPepCommandData;

typedef struct tagTUtPepResponseData
{
    UINT8 ErrorCode;
    
    union
    {
        UINT16 Version[31];
        UINT8 nData[63];
        UINT8 Padding[63];
    } Data;
} TUtPepResponseData;

#if defined(_MSC_VER)
#pragma pack(pop)
#elif defined(__XC8) || defined(__18CXX)
#else
#error Need to specify how to restore original structure padding
#endif

#endif // PepFirmwareDefs_H

/***************************************************************************/
/*  Copyright (C) 2006-2022 Kevin Eshbach                                  */
/***************************************************************************/
