/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPepDevices_H)
#define UtPepDevices_H

#include <Includes/UtExternC.h>

#if defined(_MSC_VER)
#pragma pack(push, 4)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

#define UTPEPDEVICESAPI __stdcall

#define UTPEPDEVICESCALLBACK __stdcall

typedef enum EDeviceOperation
{ 
    edoRead,
    edoWrite,
    edoProgram,
    edoVerify
} EDeviceOperation;

/*
    Device IO Functions
*/

typedef VOID (UTPEPDEVICESCALLBACK *TUtPepDevicesBeginDeviceIOFunc)(ULONG ulTotalAddresses, EDeviceOperation DeviceOperation);
typedef VOID (UTPEPDEVICESCALLBACK *TUtPepDevicesEndDeviceIOFunc)(BOOL bErrorOccurred, EDeviceOperation DeviceOperation);
typedef VOID (UTPEPDEVICESCALLBACK *TUtPepDevicesProgressDeviceIOFunc)(ULONG ulAddress);
typedef VOID (UTPEPDEVICESCALLBACK *TUtPepDevicesVerifyByteErrorDeviceIOFunc)(ULONG ulAddress, BYTE byExpected, BYTE byFound);
typedef VOID (UTPEPDEVICESCALLBACK *TUtPepDevicesVerifyWordErrorDeviceIOFunc)(ULONG ulAddress, WORD wExpected, WORD wFound);
typedef BOOL (UTPEPDEVICESCALLBACK *TUtPepDevicesContinueDeviceIOFunc)(VOID);
typedef VOID (UTPEPDEVICESCALLBACK *TUtPepDevicesShowMessageDeviceIOFunc)(LPCWSTR pszMessage);
typedef VOID (UTPEPDEVICESCALLBACK *TUtPepDevicesLogMessageDeviceIOFunc)(LPCWSTR pszMessage);

typedef struct tagTDeviceIOFuncs
{
    TUtPepDevicesBeginDeviceIOFunc pBeginDeviceIOFunc;
    TUtPepDevicesEndDeviceIOFunc pEndDeviceIOFunc;
    TUtPepDevicesProgressDeviceIOFunc pProgressDeviceIOFunc;
    TUtPepDevicesVerifyByteErrorDeviceIOFunc pVerifyByteErrorDeviceIOFunc;
    TUtPepDevicesVerifyWordErrorDeviceIOFunc pVerifyWordErrorDeviceIOFunc;
    TUtPepDevicesContinueDeviceIOFunc pContinueDeviceIOFunc;
    TUtPepDevicesShowMessageDeviceIOFunc pShowMessageDeviceIOFunc;
    TUtPepDevicesLogMessageDeviceIOFunc pLogMessageDeviceIOFunc;
} TDeviceIOFuncs;

/*
    Pin type
*/

#define CInputPinType L"Input"
#define COutputPinType L"Output"
#define CRegisteredPinType L"Registered"
#define CNotUsedPinType L"Not Used"

/*
    Pin polarity
*/

#define CActiveLowPinPolarity L"Active Low"
#define CActiveHighPinPolarity L"Active High"

/*
    Device pin configuration
*/

typedef struct tagTDevicePinConfig
{
    UINT nPin;
    LPCWSTR pszType;     /* Input, Output or Registered */
    LPCWSTR pszPolarity; /* Ignored if type is input */
} TDevicePinConfig;

/*
    Device Functions
*/

typedef BOOL (UTPEPDEVICESAPI *TUtPepDevicesInitFunc)(VOID);
typedef BOOL (UTPEPDEVICESAPI *TUtPepDevicesUninitFunc)(VOID);

typedef VOID (UTPEPDEVICESAPI *TUtPepDevicesReadDeviceFunc)(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
typedef VOID (UTPEPDEVICESAPI *TUtPepDevicesWriteDeviceFunc)(const TDeviceIOFuncs* pDeviceIOFuncs, const LPBYTE pbyData, ULONG ulDataLen);
typedef VOID (UTPEPDEVICESAPI *TUtPepDevicesProgramDeviceFunc)(const TDeviceIOFuncs* pDeviceIOFuncs, const LPBYTE pbyData, ULONG ulDataLen);
typedef VOID (UTPEPDEVICESAPI *TUtPepDevicesVerifyDeviceFunc)(const TDeviceIOFuncs* pDeviceIOFuncs, const LPBYTE pbyData, ULONG ulDataLen);

typedef ULONG (UTPEPDEVICESAPI *TUtPepDevicesGetFuseMapSizeFunc)(VOID);
typedef VOID (UTPEPDEVICESAPI *TUtPepDevicesConfigFuseMapFunc)(LPBYTE pbyData);
typedef BOOL (UTPEPDEVICESAPI *TUtPepDevicesSetDevicePinConfigsFunc)(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
typedef LPCWSTR (UTPEPDEVICESAPI *TUtPepDevicesAllocFuseMapTextFunc)(const LPBYTE pbyData);
typedef VOID (UTPEPDEVICESAPI *TUtPepDevicesFreeFuseMapTextFunc)(LPCWSTR pszFuseMapText);
typedef LPCWSTR (UTPEPDEVICESAPI *TUtPepDevicesAllocSampleFuseMapTextFunc)(VOID);
typedef VOID (UTPEPDEVICESAPI *TUtPepDevicesFreeSampleFuseMapTextFunc)(LPCWSTR pszFuseMapText);

typedef enum EDeviceType
{
    edtEPROM,
    edtEPROMWithIO,
    edtPROM,
    edtSRAM,
    edtPAL
} EDeviceType;

typedef enum EDeviceVpp
{
    edv12,
    edv12dot75,
    edv21,
    edv25,
    edv26,
    edvNotSupported
} EDeviceVpp;

typedef enum EDevicePackage
{
    edpDIP
} EDevicePackage;

/*
    Device pin description
*/

typedef struct tagTDevicePinConfigValues
{
    UINT nPin;               /* Pin number */
    LPCWSTR* ppszTypes;      /* Pin type data */
    UINT nTypesCount;        /* Pin type data count */
    LPCWSTR* ppszPolarities; /* Pin polarity data  */
    UINT nPolaritiesCount;   /* Pin polarity data count */
} TDevicePinConfigValues;

/*
    Device Pin fuse row configuration
*/

#define CNoFuseRow (ULONG)~0
#define CNoFuseColumn (ULONG)~0
#define CNoOutputEnableFuseRow (ULONG)~0

/*
    Device Pin fuse row configuration
*/

typedef struct tagTDevicePinFuseRows
{
    UINT nPin;                  /* Pin number */
    ULONG nFuseRowOutputEnable; /* Fuse row for the output enable */
    ULONG nFuseRowTermStart;    /* Fuse row for the first term */
    ULONG nFuseRowTermEnd;      /* Fuse row for the last term */
} TDevicePinFuseRows;

/*
    Device Pin fuse column configuration
*/

typedef struct tagTDevicePinFuseColumns
{
    UINT nPin;             /* Pin number */
    ULONG nLowFuseColumn;  /* Column number for low output */
    ULONG nHighFuseColumn; /* Column number for high output */
} TDevicePinFuseColumns;

/*
   Device specific data structures
*/

typedef struct tagTROMData
{
    UINT nSize; /* Size of the device in bytes */
    UINT nBitsPerValue;
    EDeviceVpp DeviceVpp;
    TUtPepDevicesReadDeviceFunc pReadDeviceFunc;
    TUtPepDevicesProgramDeviceFunc pProgramDeviceFunc;
    TUtPepDevicesVerifyDeviceFunc pVerifyDeviceFunc;
} TROMData;

typedef struct tagTRAMData
{
    UINT nSize; /* Size of the device in bytes */
    UINT nBitsPerValue;
    TUtPepDevicesWriteDeviceFunc pWriteDeviceFunc;
    TUtPepDevicesVerifyDeviceFunc pVerifyDeviceFunc;
} TRAMData;

typedef struct tagTPALData
{
    TDevicePinConfigValues* pDevicePinConfigValues;
    UINT nDevicePinConfigValuesCount;
    TDevicePinFuseRows* pDevicePinFuseRows;
    UINT nDevicePinFuseRowsCount;
    TDevicePinFuseColumns* pDevicePinFuseColumns;
    UINT nDevicePinFuseColumnsCount;
    TUtPepDevicesGetFuseMapSizeFunc pGetFuseMapSizeFunc;
    TUtPepDevicesConfigFuseMapFunc pConfigFuseMapFunc;
    TUtPepDevicesSetDevicePinConfigsFunc pSetDevicePinConfigsFunc;
    TUtPepDevicesAllocFuseMapTextFunc pAllocFuseMapTextFunc;
    TUtPepDevicesFreeFuseMapTextFunc pFreeFuseMapTextFunc;
    TUtPepDevicesAllocSampleFuseMapTextFunc pAllocSampleFuseMapTextFunc;
    TUtPepDevicesFreeSampleFuseMapTextFunc pFreeSampleFuseMapTextFunc;
    TUtPepDevicesReadDeviceFunc pReadDeviceFunc;
} TPALData;

typedef struct tagTDeviceData
{
    TUtPepDevicesInitFunc pInitFunc;
    TUtPepDevicesUninitFunc pUninitFuncs;
    TROMData ROMData;
    TRAMData RAMData;
    TPALData PALData;
} TDeviceData;

/*
   Device structure
*/

typedef struct tagTDevice
{
    LPCWSTR pszName;                 /* Name of the device */
    EDeviceType DeviceType;          /* Type of device */
    EDevicePackage DevicePackage;    /* Package of the device */
    UINT nPinCount;                  /* Number of pins on the device */
    LPCWSTR* ppszPinNames;           /* Pin names of the device */
    UINT nPinNamesCount;             /* Count of the pin names for the device */
    LPCWSTR pszAdapter;              /* Adapter used for the device */
    LPCWSTR pszMessage;              /* Device message */
    BOOL bDipSwitches[8];            /* Dip switch settings */
	UINT32 nChipEnableNanoseconds;   /* number of nanoseconds before chip ready after VCC applied */
	UINT32 nOutputEnableNanoseconds; /* number of nanoseconds before data available after the Output Enable goes low */
    TDeviceData DeviceData;          /* Device specific data */
} TDevice;

/*
    Device macros
*/

#define DEVICES_BEGIN \
static TDevice l_Devices[] = {

#define DEVICES_END \
    {0}}; \
    VOID UTPEPDEVICESAPI GetDeviceCount( \
      PULONG pulDeviceCount) \
    { \
        *pulDeviceCount = MArrayLen(l_Devices) - 1; \
    } \
    BOOL UTPEPDEVICESAPI GetDevice( \
      ULONG ulIndex, \
      TDevice* pDevice) \
    { \
        if (ulIndex >= MArrayLen(l_Devices) - 1) \
        { \
            return FALSE; \
        } \
        CopyMemory(pDevice, &l_Devices[ulIndex], sizeof(TDevice)); \
        return TRUE; \
    }

#define DEVICE_DATA_BEGIN(name) \
    {L#name,

#define DEVICE_DATA_END \
    },

/*
    Device type macros
*/

#define DEVICE_EPROM \
    edtEPROM,

#define DEVICE_EPROMWithIO \
    edtEPROMWithIO,

#define DEVICE_PROM \
    edtPROM,

#define DEVICE_SRAM \
    edtSRAM,

#define DEVICE_PAL \
    edtPAL,

/*
    Device data macros
*/

#define DEVICE_DIP_PACKAGE \
    edpDIP,

#define DEVICE_PINCOUNT(pinCount) \
    pinCount,

#define DEVICE_PINNAMES(pinNames) \
    pinNames, MArrayLen(pinNames),

#define DEVICE_ADAPTER(adapter) \
    L##adapter, \

#define DEVICE_ADAPTER_NONE \
    NULL, \

#define DEVICE_MESSAGE(message) \
    L##message,

#define DEVICE_MESSAGE_NONE \
    NULL,

#define DEVICE_DIPSWITCHES(dip1, dip2, dip3, dip4, dip5, dip6, dip7, dip8) \
    {dip1, dip2, dip3, dip4, dip5, dip6, dip7, dip8},

#define DEVICE_CHIP_ENABLE_NANO_SECS(ns) \
	ns,

#define DEVICE_CHIP_ENABLE_NANO_SECS_NONE \
	0,

#define DEVICE_OUTPUT_ENABLE_NANO_SECS(ns) \
	ns,

#define DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE \
	0,

#define DEVICE_INIT_FUNC(initFunc) \
    {initFunc,

#define DEVICE_UNINIT_FUNC(uninitFunc) \
    uninitFunc,

/*
    ROM device data macros
*/

#define DEVICE_ROM_DATA_BEGIN \
    {

#define DEVICE_ROM_DATA_END \
    }, {0}, {0} }

#define DEVICE_ROM_SIZE(sizeBytes) \
    sizeBytes,

#define DEVICE_ROM_BITSPERVALUE(bitsPerValue) \
    bitsPerValue,

#define DEVICE_ROM_VPP_12VDC \
    edv12,

#define DEVICE_ROM_VPP_12dot75VDC \
    edv12dot75,

#define DEVICE_ROM_VPP_21VDC \
    edv21,

#define DEVICE_ROM_VPP_25VDC \
    edv25,

#define DEVICE_ROM_VPP_26VDC \
    edv26,

#define DEVICE_ROM_VPP_NOTSUPPORTED \
    edvNotSupported,

#define DEVICE_ROM_READ_FUNC(readFunc) \
    readFunc,

#define DEVICE_ROM_PROGRAM_FUNC(programFunc) \
    programFunc,

#define DEVICE_ROM_PROGRAM_FUNC_NONE \
    NULL,

#define DEVICE_ROM_VERIFY_FUNC(verifyFunc) \
    verifyFunc

/*
    RAM device data macros
*/

#define DEVICE_RAM_DATA_BEGIN \
    {0}, {

#define DEVICE_RAM_DATA_END \
    }, {0} }

#define DEVICE_RAM_SIZE(sizeBytes) \
    sizeBytes,

#define DEVICE_RAM_BITSPERVALUE(bitsPerValue) \
    bitsPerValue,

#define DEVICE_RAM_WRITE_FUNC(writeFunc) \
    writeFunc,

#define DEVICE_RAM_VERIFY_FUNC(verifyFunc) \
    verifyFunc

/*
    PAL device data macros
*/

#define DEVICE_PAL_DATA_BEGIN \
    {0}, {0}, {

#define DEVICE_PAL_DATA_END \
    } }

#define DEVICE_PAL_PINCONFIGVALUES(pinConfigValues) \
    pinConfigValues, MArrayLen(pinConfigValues),

#define DEVICE_PAL_PINFUSEROWS(pinFuseRows) \
    pinFuseRows, MArrayLen(pinFuseRows),

#define DEVICE_PAL_PINFUSECOLUMNS(pinFuseColumns) \
    pinFuseColumns, MArrayLen(pinFuseColumns),

#define DEVICE_PAL_GETFUSEMAPSIZE_FUNC(getFuseMapSizeFunc) \
    getFuseMapSizeFunc,

#define DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE \
    NULL,

#define DEVICE_PAL_CONFIGFUSEMAP_FUNC(configFuseMapFunc) \
    configFuseMapFunc,

#define DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE \
    NULL,

#define DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(setDevicePinConfigsFunc) \
    setDevicePinConfigsFunc,

#define DEVICE_PAL_FUSEMAPTEXT_FUNCS(allocFuseMapText, freeFuseMapText) \
    allocFuseMapText, freeFuseMapText,

#define DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE \
    NULL, NULL,

#define DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS(allocSampleFuseMapText, freeSampleFuseMapText) \
    allocSampleFuseMapText, freeSampleFuseMapText,

#define DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE \
    NULL, NULL,

#define DEVICE_PAL_READ_FUNC(readFunc) \
    readFunc

/*
    Device pin description macro
*/

#define MDevicePinConfigValues(pin, pinTypeValues, pinPolarityValues) \
    {pin, pinTypeValues, MArrayLen(pinTypeValues), pinPolarityValues, MArrayLen(pinPolarityValues)}

#define MDevicePinConfigValuesNoPolarity(pin, pinConfigValues) \
    {pin, pinConfigValues, MArrayLen(pinConfigValues), NULL, 0}

MExternC BOOL UTPEPDEVICESAPI UtPepDevicesInitialize(LPCWSTR pszPluginPath);
MExternC BOOL UTPEPDEVICESAPI UtPepDevicesUninitialize(VOID);
MExternC BOOL UTPEPDEVICESAPI UtPepDevicesGetDeviceCount(PULONG pulDeviceCount);
MExternC BOOL UTPEPDEVICESAPI UtPepDevicesGetDevice(ULONG ulIndex, TDevice* pDevice);
MExternC BOOL UTPEPDEVICESAPI UtPepDevicesFindDevice(LPCWSTR pszName, EDeviceType DeviceType, TDevice* pDevice);

MExternC BOOL UTPEPDEVICESAPI UtPepDevicesGetPluginCount(PULONG pulPluginCount);
MExternC BOOL UTPEPDEVICESAPI UtPepDevicesGetPluginName(ULONG ulIndex, LPCWSTR* ppszName);
MExternC BOOL UTPEPDEVICESAPI UtPepDevicesGetPluginVersion(ULONG ulIndex, PWORD pwProductMajorVersion, PWORD pwProductMinorVersion, PWORD pwProductBuildVersion, PWORD pwProductPrivateVersion);

MExternC LPCWSTR UTPEPDEVICESAPI UtPepDevicesGetDeviceTypeName(EDeviceType DeviceType);
MExternC LPCWSTR UTPEPDEVICESAPI UtPepDevicesGetDevicePackageName(EDevicePackage DevicePackage);
MExternC LPCWSTR UTPEPDEVICESAPI UtPepDevicesGetDeviceVppName(EDeviceVpp DeviceVpp);

MExternC LPCWSTR UTPEPDEVICESAPI UtPepDevicesAllocPinDiagram(EDevicePackage DevicePackage, LPCWSTR* ppszPinNames, UINT nPinNamesCount);
MExternC BOOL UTPEPDEVICESAPI UtPepDevicesFreePinDiagram(LPCWSTR pszPinDiagram);

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* UtPepDevices_H */

/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/
