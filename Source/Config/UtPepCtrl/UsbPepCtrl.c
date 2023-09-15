/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <strsafe.h>

#include <setupapi.h>

#include <hidsdi.h>

#include <winusb.h>

#include <dbt.h>

#include <Config/UtPepCtrl.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeap.h>

#include <Firmware/PepFirmwareDefs.h>

#include <UtilsPep/UtPepLogicDefs.h>

#include "UtPepCtrlUtil.h"

#include "UsbDeviceInterfaceGUID.h"

#pragma region "Constants"

#define CUiDeviceNotificationCtrlClass L"DeviceNotificationCtrl"

#define CHidLibrary L"hid.dll"
#define CSetupApiLibrary L"setupapi.dll"
#define CWinUsbLibrary L"winusb.dll"

#define CDefSendTimeoutMs 1000
#define CDefReceiveTimeoutMs 1000

#define CDefCancelOverlappedTimeoutMs 1000

#define CReadPipeID (1 | 0x80) // 0x80 - IN endpoint
#define CWritePipeID 1

#define CReceiveRetryCount 3

#pragma endregion

#pragma region "Enumerations"

typedef enum tagEUsbDeviceType
{
    eUsbInvalidDeviceType,
    eUsbHIDDeviceType,
    eUsbFullSpeedDeviceType
} EUsbDeviceType;

#pragma endregion

#pragma region "Type Definitions"

typedef BOOLEAN (__stdcall *TGetPreparsedDataFunc)(_In_ HANDLE HidDeviceObject, _Out_ PHIDP_PREPARSED_DATA* PreparsedData);
typedef NTSTATUS (__stdcall *TGetCapsFunc)(_In_ PHIDP_PREPARSED_DATA PreparsedData, _Out_ PHIDP_CAPS Capabilities);
typedef BOOLEAN (__stdcall *TFreePreparsedDataFunc)(_In_ PHIDP_PREPARSED_DATA PreparsedData);
typedef BOOLEAN (__stdcall *TGetAttributesFunc)(_In_ HANDLE HidDeviceObject, _Out_ PHIDD_ATTRIBUTES Attributes);

typedef BOOL (__stdcall *TWinUsbInitializeFunc)(_In_ HANDLE DeviceHandle, _Out_ PWINUSB_INTERFACE_HANDLE InterfaceHandle);
typedef BOOL (__stdcall *TWinUsbFreeFunc)(_In_ WINUSB_INTERFACE_HANDLE InterfaceHandle);
typedef BOOL (__stdcall *TWinUsbReadPipeFunc)(_In_ WINUSB_INTERFACE_HANDLE InterfaceHandle, _In_ UCHAR PipeID, _Out_writes_bytes_to_opt_(BufferLength, *LengthTransferred) PUCHAR Buffer, _In_ ULONG BufferLength, _Out_opt_ PULONG LengthTransferred, _In_opt_ LPOVERLAPPED Overlapped);
typedef BOOL (__stdcall *TWinUsbWritePipeFunc)(_In_ WINUSB_INTERFACE_HANDLE InterfaceHandle, _In_ UCHAR PipeID, _In_reads_bytes_(BufferLength) PUCHAR Buffer, _In_ ULONG BufferLength, _Out_opt_ PULONG LengthTransferred, _In_opt_ LPOVERLAPPED Overlapped);
typedef BOOL (__stdcall *TWinUsbGetOverlappedResultFunc)(_In_ WINUSB_INTERFACE_HANDLE InterfaceHandle, _In_ LPOVERLAPPED lpOverlapped, _Out_ LPDWORD lpNumberOfBytesTransferred, _In_ BOOL bWait);
typedef BOOL (__stdcall *TWinUsb_AbortPipeFunc)(_In_ WINUSB_INTERFACE_HANDLE InterfaceHandle, _In_ UCHAR PipeID);

typedef HDEVINFO (WINAPI *TSetupDiGetClassDevsFunc)(_In_opt_ CONST GUID* ClassGuid, _In_opt_ PCWSTR Enumerator, _In_opt_ HWND hwndParent, _In_ DWORD Flags);
typedef BOOL (WINAPI *TSetupDiEnumDeviceInterfacesFunc)(_In_ HDEVINFO DeviceInfoSet, _In_opt_ PSP_DEVINFO_DATA DeviceInfoData, _In_ CONST GUID* InterfaceClassGuid, _In_ DWORD MemberIndex, _Out_ PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData);
typedef BOOL (WINAPI *TSetupDiGetDeviceInterfaceDetailFunc)(_In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData, _Out_writes_bytes_to_opt_(DeviceInterfaceDetailDataSize, *RequiredSize) PSP_DEVICE_INTERFACE_DETAIL_DATA_W DeviceInterfaceDetailData, _In_ DWORD DeviceInterfaceDetailDataSize, _Out_opt_ _Out_range_(>= , sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W)) PDWORD RequiredSize, _Out_opt_ PSP_DEVINFO_DATA DeviceInfoData);
typedef BOOL (WINAPI *TSetupDiDestroyDeviceInfoListFunc)(_In_ HDEVINFO DeviceInfoSet);

#pragma endregion

#pragma region "Structures"

typedef struct tagTUsbGuidTypeData {
    GUID Guid;
    EUsbDeviceType UsbDeviceType;
} TUsbGuidTypeData;

typedef struct tagTUsbHidDeviceData {
    INT nInputReportLen;
} TUsbHidDeviceData;

typedef struct tagTUsbFullSpeedDeviceData {
    HMODULE hModule;
    TWinUsbInitializeFunc pInitializeFunc;
    TWinUsbFreeFunc pFreeFunc;
    TWinUsbReadPipeFunc pReadPipeFunc;
    TWinUsbWritePipeFunc pWritePipeFunc;
    TWinUsbGetOverlappedResultFunc pGetOverlappedResultFunc;
    TWinUsb_AbortPipeFunc pAbortPipeFunc;
    WINUSB_INTERFACE_HANDLE InterfaceHandle;
} TUsbFullSpeedDeviceData;

typedef struct tagUsbOverlappedData {
    OVERLAPPED Overlapped;

    union {
        TUtPepCommandData CommandData;
        TUtPepResponseData ResponseData;
    } Data;
} TUsbOverlappedData;

typedef struct tagTUsbDeviceData {
    HANDLE* phDeviceMutex;
    HANDLE* phDeviceChangeListMutex;
    EUtPepCtrlDeviceChange** ppPepCtrlDeviceChange;
    LPINT pnPepCtrlDeviceChangeLen;
    HANDLE hDevice;
    GUID DeviceInterfaceGuid;
    LPWSTR pszDeviceName;

    TUsbOverlappedData* pReadOverlappedData;
    TUsbOverlappedData* pWriteOverlappedData;

    union {
        TUsbHidDeviceData UsbHidDeviceData;
        TUsbFullSpeedDeviceData UsbFullSpeedDeviceData;
    } DeviceData;
} TUsbDeviceData;

typedef struct tagTDispatchDeviceChangeData {
    TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc;
    HANDLE hQuitEvent;
    HANDLE hDeviceChangeListMutex;
    EUtPepCtrlDeviceChange* pPepCtrlDeviceChange;
    INT nPepCtrlDeviceChangeLen;
} TDispatchDeviceChangeData;

typedef struct tagTDeviceChangeData 
{
    HANDLE hQuitEvent;
    HANDLE hThreadInitializedEvent;
    BOOL bThreadInitialized;
    TUsbDeviceData* pUsbDeviceData;
    TDispatchDeviceChangeData* pDispatchDeviceChangeData;
} TDeviceChangeData;

typedef struct tagTDeviceData
{
    HANDLE hDispatchDeviceChangeThread;
    HANDLE hDeviceChangeThread;
    HANDLE hDeviceMutex;
    TUsbDeviceData UsbDeviceData;
    TDispatchDeviceChangeData DispatchDeviceChangeData;
} TDeviceData;

#pragma endregion

#pragma region "Local Variables"

static TUsbGuidTypeData l_UsbGuidTypeData[2] = {0};

static TDeviceData l_DeviceData = {0};

static TDeviceChangeData l_DeviceChangeData = {0};

static UINT8 l_nCurrentPacketID = 0;

#if !defined(NDEBUG)
static INT l_nLogCount = 0;
#endif

#pragma endregion

#pragma region "Local Functions"

#if !defined(NDEBUG)

static VOID lDumpVersionCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100];

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID: #%d: Version Command\r\n",
                     pCommandData->PacketID, l_nLogCount);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpResetCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100];

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID: #%d: Reset Command\r\n",
                     pCommandData->PacketID, l_nLogCount);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpSetProgrammerModeCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100], cValue[12];
    LPCWSTR pszDescription;

    switch (pCommandData->Data.nProgrammerMode)
    {
        case CUtPepLogicProgrammerNoneMode:
            pszDescription = L"None";
            break;
        case CUtPepLogicProgrammerReadMode:
            pszDescription = L"Read";
            break;
        case CUtPepLogicProgrammerWriteMode:
            pszDescription = L"Write";
            break;
        default:
            StringCchPrintfW(cValue, MArrayLen(cValue), L"0x%08X", pCommandData->Data.nProgrammerMode);

            pszDescription = cValue;
            break;
    }

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID: #%d: Set Programmer Mode Command (%s)\r\n",
                     pCommandData->PacketID, l_nLogCount, pszDescription);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpSetVccModeCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100], cValue[12];
    LPCWSTR pszDescription;

    switch (pCommandData->Data.nVccMode)
    {
        case CUtPepLogic5VDCMode:
            pszDescription = L"+5VDC";
            break;
        case CUtPepLogic625VDCMode:
            pszDescription = L"+6.25VDC";
            break;
        default:
            StringCchPrintfW(cValue, MArrayLen(cValue), L"0x%08X", pCommandData->Data.nVccMode);

            pszDescription = cValue;
            break;
    }

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, PacketID: #%d: Set Vcc Mode Command (%s)\r\n",
                     pCommandData->PacketID, l_nLogCount, pszDescription);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpSetPinPulseModeCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100], cValue[12];
    LPCWSTR pszDescription;

    switch (pCommandData->Data.nPinPulseMode)
    {
        case CUtPepLogicPinPulse1Mode:
            pszDescription = L"Pin Pulse 1";
            break;
        case CUtPepLogicPinPulse2Mode:
            pszDescription = L"Pin Pulse 2";
            break;
        case CUtPepLogicPinPulse3Mode:
            pszDescription = L"Pin Pulse 3";
            break;
        case CUtPepLogicPinPulse4Mode:
            pszDescription = L"Pin Pulse 4";
            break;
        default:
            StringCchPrintfW(cValue, MArrayLen(cValue), L"0x%08X", pCommandData->Data.nPinPulseMode);

            pszDescription = cValue;
            break;
    }

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, PacketID #%d: Set Pin Pulse Mode Command (%s)\r\n",
                     pCommandData->PacketID, l_nLogCount, pszDescription);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpSetVppModeCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100], cValue[12];
    LPCWSTR pszDescription;

    switch (pCommandData->Data.nVppMode)
    {
        case CUtPepLogic12VDCVppMode:
            pszDescription = L"+12VDC";
            break;
        case CUtPepLogic21VDCVppMode:
            pszDescription = L"+21VDC";
            break;
        case CUtPepLogic25VDCVppMode:
            pszDescription = L"+25VDC";
            break;
        default:
            StringCchPrintfW(cValue, MArrayLen(cValue), L"0x08%X", pCommandData->Data.nVppMode);

            pszDescription = cValue;
            break;
    }

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID #%d: Set Vpp Mode Command (%s)\r\n",
                     pCommandData->PacketID, l_nLogCount, pszDescription);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpReadDataCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100];

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID #%d: Read Data Command (Address: 0x%08X, Data Len: %d)\r\n",
                     pCommandData->PacketID,
                     l_nLogCount,
                     pCommandData->Data.ReadData.nAddress,
                     pCommandData->Data.ReadData.nDataLen);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpReadUserDataCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100];

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID #%d: Read User Data Command (Data Len: %d)\r\n",
                     pCommandData->PacketID,
                     l_nLogCount,
                     pCommandData->Data.ReadUserData.nDataLen);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpReadUserDataWithDelayCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100];

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID #%d: Read User Data With Delay Command (Data Len: %d)\r\n",
                     pCommandData->PacketID,
                     l_nLogCount,
                     pCommandData->Data.ReadUserDataWithDelay.nDataLen);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpProgramDataCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100];

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID #%d: Program Data Command (Data Len: %d)\r\n",
                     pCommandData->PacketID,
                     l_nLogCount,
                     pCommandData->Data.ProgramData.nDataLen);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpProgramUserDataCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100];

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID #%d: Program User Data Command (Data Len: %d)\r\n",
                     pCommandData->PacketID,
                     l_nLogCount,
                     pCommandData->Data.ProgramUserData.nDataLen);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpSetDelaysCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[150];

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID #%d: Set Delays Command (Chip Enable Nano Seconds: %d, Output Enable Nano Seconds: %d)\r\n",
                     pCommandData->PacketID,
                     l_nLogCount,
                     pCommandData->Data.Delays.nChipEnableNanoSeconds,
                     pCommandData->Data.Delays.nOutputEnableNanoSeconds);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpUnknownCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    WCHAR cBuffer[100];

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID #%d: Unknown Command (0x02%X)\r\n",
                     pCommandData->PacketID, l_nLogCount, pCommandData->Command);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpCommandData(
  _In_ const TUtPepCommandData* pCommandData)
{
    ++l_nLogCount;

    switch (pCommandData->Command)
    {
        case CPepVersionCommand:
            lDumpVersionCommandData(pCommandData);
            break;
        case CPepResetCommand:
            lDumpResetCommandData(pCommandData);
            break;
        case CPepSetProgrammerModeCommand:
            lDumpSetProgrammerModeCommandData(pCommandData);
            break;
        case CPepSetVccModeCommand:
            lDumpSetVccModeCommandData(pCommandData);
            break;
        case CPepSetPinPulseModeCommand:
            lDumpSetPinPulseModeCommandData(pCommandData);
            break;
        case CPepSetVppModeCommand:
            lDumpSetVppModeCommandData(pCommandData);
            break;
        case CPepReadDataCommand:
            lDumpReadDataCommandData(pCommandData);
            break;
        case CPepReadUserDataCommand:
            lDumpReadUserDataCommandData(pCommandData);
            break;
        case CPepReadUserDataWithDelayCommand:
            lDumpReadUserDataWithDelayCommandData(pCommandData);
            break;
        case CPepProgramDataCommand:
            lDumpProgramDataCommandData(pCommandData);
            break;
        case CPepProgramUserDataCommand:
            lDumpProgramUserDataCommandData(pCommandData);
            break;
        case CPepSetDelaysCommand:
            lDumpSetDelaysCommandData(pCommandData);
            break;
        default:
            lDumpUnknownCommandData(pCommandData);
            break;
    }
}

static VOID lDumpResponseData(
  _In_ const TUtPepResponseData* pResponseData)
{
    WCHAR cBuffer[260], cValue[20];
    LPCWSTR pszDescription;

    switch (pResponseData->ErrorCode)
    {
        case CPepErrorSuccess:
            pszDescription = L"Success";
            break;
        case CPepErrorInvalidCommand:
            pszDescription = L"Invalid Command";
            break;
        case CPepErrorFailed:
            pszDescription = L"Failed";
            break;
        case CPepErrorInvalidLength:
            pszDescription = L"Invalid Length";
            break;
        case CPepErrorBusy:
            pszDescription = L"Busy";
            break;
        case CPepErrorInitializeData:
            pszDescription = L"Initialize Data";
            break;
        default:
            StringCchPrintfW(cValue, MArrayLen(cValue), L"0x%02X",
                             pResponseData->ErrorCode);

            pszDescription = cValue;
            break;
    }

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d, Packet ID #%d: Error Code Response (%s), Data (",
                     pResponseData->PacketID, l_nLogCount, pszDescription);

    for (INT nIndex = 0; nIndex < MArrayLen(pResponseData->Data.nData); ++nIndex)
    {
        if (nIndex > 0)
        {
            StringCchCat(cBuffer, MArrayLen(cBuffer), L" ");
        }

        StringCchPrintfW(cValue, MArrayLen(cValue), L"%02X",
                         pResponseData->Data.nData[nIndex]);

        StringCchCat(cBuffer, MArrayLen(cBuffer), cValue);
    }

    StringCchCat(cBuffer, MArrayLen(cBuffer), L")\r\n");

    OutputDebugStringW(cBuffer);
}

static VOID lDumpMessage(
  _In_ LPCWSTR pszMessage)
{
    WCHAR cBuffer[100];

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl #%d: %s\r\n",
                     l_nLogCount, pszMessage);

    OutputDebugStringW(cBuffer);
}

static VOID lDumpNonMatchingPacketID(
  _In_ const TUtPepCommandData* pCommandData,
  _In_ const TUtPepResponseData* pResponseData)
{
    WCHAR cBuffer[100];

    StringCchPrintfW(cBuffer, MArrayLen(cBuffer), L"UsbPepCtrl: Command Data Packet ID #%d does not match Response Data Packet ID #%d.\r\n",
                     pCommandData->PacketID, pResponseData->PacketID);

    OutputDebugStringW(cBuffer);
}

#endif

static VOID lIncrementPacketID(
  _In_ TUtPepCommandData* pCommandData)
{
    if (l_nCurrentPacketID < 255)
    {
        ++l_nCurrentPacketID;
    }
    else
    {
        l_nCurrentPacketID = 0;
    }

    pCommandData->PacketID = l_nCurrentPacketID;
}

static VOID lInitializeGuidTypeData(VOID)
{
    for (INT nIndex = 0; nIndex < MArrayLen(l_UsbGuidTypeData); ++nIndex)
    {
        l_UsbGuidTypeData[nIndex].UsbDeviceType = eUsbInvalidDeviceType;

        ZeroMemory(&l_UsbGuidTypeData[nIndex].Guid, sizeof(GUID));
    }

    if (UsbHIDDeviceInterfaceGUID(&l_UsbGuidTypeData[0].Guid))
    {
        l_UsbGuidTypeData[0].UsbDeviceType = eUsbHIDDeviceType;
    }

    if (UsbFullSpeedDeviceInterfaceGUID(&l_UsbGuidTypeData[1].Guid))
    {
        l_UsbGuidTypeData[1].UsbDeviceType = eUsbFullSpeedDeviceType;
    }
}

static EUsbDeviceType lFindUsbDeviceType(
  _Const_ _In_ LPCGUID pGuid)
{
    for (INT nIndex = 0; nIndex < MArrayLen(l_UsbGuidTypeData); ++nIndex)
    {
        if (l_UsbGuidTypeData[nIndex].UsbDeviceType == eUsbInvalidDeviceType)
        {
            continue;
        }

        if (IsEqualGUID(&l_UsbGuidTypeData[nIndex].Guid, pGuid))
        {
            return l_UsbGuidTypeData[nIndex].UsbDeviceType;
        }
    }

    return eUsbInvalidDeviceType;
}

_Success_(return)
static BOOL lInitializeUsbFullSpeedDeviceData(
  _In_ TUsbFullSpeedDeviceData* pDeviceData)
{
    pDeviceData->hModule = LoadLibraryW(CWinUsbLibrary);

    if (pDeviceData->hModule == NULL)
    {
        return FALSE;
    }

    pDeviceData->pInitializeFunc = (TWinUsbInitializeFunc)GetProcAddress(pDeviceData->hModule, "WinUsb_Initialize");
    pDeviceData->pFreeFunc = (TWinUsbFreeFunc)GetProcAddress(pDeviceData->hModule, "WinUsb_Free");
    pDeviceData->pReadPipeFunc = (TWinUsbReadPipeFunc)GetProcAddress(pDeviceData->hModule, "WinUsb_ReadPipe");
    pDeviceData->pWritePipeFunc = (TWinUsbWritePipeFunc)GetProcAddress(pDeviceData->hModule, "WinUsb_WritePipe");
    pDeviceData->pGetOverlappedResultFunc = (TWinUsbGetOverlappedResultFunc)GetProcAddress(pDeviceData->hModule, "WinUsb_GetOverlappedResult");
    pDeviceData->pAbortPipeFunc = (TWinUsb_AbortPipeFunc)GetProcAddress(pDeviceData->hModule, "WinUsb_AbortPipe");


    if (pDeviceData->pInitializeFunc && 
        pDeviceData->pFreeFunc &&
        pDeviceData->pReadPipeFunc &&
        pDeviceData->pWritePipeFunc &&
        pDeviceData->pGetOverlappedResultFunc &&
        pDeviceData->pAbortPipeFunc)
    {
        return TRUE;
    }

    FreeLibrary(pDeviceData->hModule);

    ZeroMemory(pDeviceData, sizeof(TUsbFullSpeedDeviceData));

    return FALSE;
}

static VOID lUninitializeUsbFullSpeedDeviceData(
  _In_ TUsbFullSpeedDeviceData* pDeviceData)
{
    FreeLibrary(pDeviceData->hModule);

    ZeroMemory(pDeviceData, sizeof(TUsbFullSpeedDeviceData));
}

static VOID lCloseAll(VOID)
{
    if (l_DeviceData.hDeviceChangeThread)
    {
        SetEvent(l_DeviceChangeData.hQuitEvent);

        WaitForSingleObject(l_DeviceData.hDeviceChangeThread, INFINITE);

        CloseHandle(l_DeviceData.hDeviceChangeThread);

        l_DeviceData.hDeviceChangeThread = NULL;
    }

    if (l_DeviceChangeData.hQuitEvent)
    {
        CloseHandle(l_DeviceChangeData.hQuitEvent);

        l_DeviceChangeData.hQuitEvent = NULL;
    }

    if (l_DeviceData.hDispatchDeviceChangeThread)
    {
        SetEvent(l_DeviceData.DispatchDeviceChangeData.hQuitEvent);

        WaitForSingleObject(l_DeviceData.hDispatchDeviceChangeThread, INFINITE);

        CloseHandle(l_DeviceData.hDispatchDeviceChangeThread);

        l_DeviceData.hDispatchDeviceChangeThread = NULL;
    }

    if (l_DeviceData.DispatchDeviceChangeData.hQuitEvent)
    {
        CloseHandle(l_DeviceData.DispatchDeviceChangeData.hQuitEvent);

        l_DeviceData.DispatchDeviceChangeData.hQuitEvent = NULL;
    }

    if (l_DeviceData.DispatchDeviceChangeData.hDeviceChangeListMutex)
    {
        CloseHandle(l_DeviceData.DispatchDeviceChangeData.hDeviceChangeListMutex);

        l_DeviceData.DispatchDeviceChangeData.hDeviceChangeListMutex = NULL;
    }

    if (l_DeviceData.DispatchDeviceChangeData.pPepCtrlDeviceChange)
    {
        UtFreeMem(l_DeviceData.DispatchDeviceChangeData.pPepCtrlDeviceChange);

        l_DeviceData.DispatchDeviceChangeData.pPepCtrlDeviceChange = NULL;
    }

    if (l_DeviceData.hDeviceMutex)
    {
        CloseHandle(l_DeviceData.hDeviceMutex);

        l_DeviceData.hDeviceMutex = NULL;
    }
}

static VOID lPushPepCtrlDeviceChange(
  _In_ HANDLE hDeviceChangeListMutex,
  EUtPepCtrlDeviceChange** ppPepCtrlDeviceChange,
  LPINT pnPepCtrlDeviceChangeLen,
  _In_ EUtPepCtrlDeviceChange PepCtrlDeviceChange)
{
    EUtPepCtrlDeviceChange* pPepCtrlDeviceChange;

    WaitForSingleObject(hDeviceChangeListMutex, INFINITE);

    pPepCtrlDeviceChange = UtReAllocMem(*ppPepCtrlDeviceChange,
                                        (*pnPepCtrlDeviceChangeLen + 1) *
                                            sizeof(EUtPepCtrlDeviceChange));

    if (pPepCtrlDeviceChange)
    {
        pPepCtrlDeviceChange[*pnPepCtrlDeviceChangeLen] = PepCtrlDeviceChange;

        *pnPepCtrlDeviceChangeLen += 1;

        *ppPepCtrlDeviceChange = pPepCtrlDeviceChange;
    }

    ReleaseMutex(hDeviceChangeListMutex);
}

static DWORD WINAPI lDispatchDeviceChangeThreadProc(
  _In_ LPVOID pvParameter)
{
    TDispatchDeviceChangeData* pDispatchDeviceChangeData = (TDispatchDeviceChangeData*)pvParameter;
    BOOL bQuit = FALSE;
    BOOL bQuitList;
    EUtPepCtrlDeviceChange PepCtrlDeviceChange;

    while (!bQuit)
    {
        if (WaitForSingleObject(pDispatchDeviceChangeData->hQuitEvent, 100) == WAIT_OBJECT_0)
        {
            bQuit = TRUE;
        }

        bQuitList = FALSE;

        while (!bQuitList)
        {
            WaitForSingleObject(pDispatchDeviceChangeData->hDeviceChangeListMutex, INFINITE);

            if (pDispatchDeviceChangeData->nPepCtrlDeviceChangeLen == 0)
            {
                bQuitList = TRUE;

                PepCtrlDeviceChange = eUtPepCtrlDeviceArrived; // fix C4701 compiler warning
            }
            else
            {
                PepCtrlDeviceChange = pDispatchDeviceChangeData->pPepCtrlDeviceChange[0];

                --pDispatchDeviceChangeData->nPepCtrlDeviceChangeLen;

                CopyMemory(pDispatchDeviceChangeData->pPepCtrlDeviceChange,
                           pDispatchDeviceChangeData->pPepCtrlDeviceChange + 1,
                           pDispatchDeviceChangeData->nPepCtrlDeviceChangeLen * 
                               sizeof(EUtPepCtrlDeviceChange));
            }

            ReleaseMutex(pDispatchDeviceChangeData->hDeviceChangeListMutex);

            if (!bQuitList)
            {
                pDispatchDeviceChangeData->pDeviceChangeFunc(PepCtrlDeviceChange);
            }
        }
    }

    return 0;
}

static HANDLE lOpenHidDevice(
  _Const_ _In_z_ LPCWSTR pszPhysicalDeviceObjectPath)
{
    HMODULE hModule = LoadLibraryW(CHidLibrary);
    TGetAttributesFunc pGetAttributesFunc;
    HANDLE hDevice;
    HIDD_ATTRIBUTES Attributes;

    if (hModule == NULL)
    {
        return NULL;
    }

    pGetAttributesFunc = (TGetAttributesFunc)GetProcAddress(hModule, "HidD_GetAttributes");

    if (pGetAttributesFunc == NULL)
    {
        FreeLibrary(hModule);

        return FALSE;
    }

    hDevice = CreateFile(pszPhysicalDeviceObjectPath, GENERIC_READ | GENERIC_WRITE,
                         0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        FreeLibrary(hModule);

        return NULL;
    }

    Attributes.Size = sizeof(Attributes);

    if (pGetAttributesFunc(hDevice, &Attributes))
    {
        if (Attributes.VendorID == CPepFirmwareVendorID &&
            Attributes.ProductID == CPepFirmwareProductID)
        {
            FreeLibrary(hModule);

            return hDevice;
        }
    }

    CloseHandle(hDevice);

    FreeLibrary(hModule);

    return NULL;
}

static HANDLE lOpenFullSpeedDevice(
  _Const_ _In_z_ LPCWSTR pszPhysicalDeviceObjectPath)
{
    HANDLE hDevice;

    hDevice = CreateFile(pszPhysicalDeviceObjectPath, GENERIC_READ | GENERIC_WRITE,
                         0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    return hDevice;
}

static VOID lCloseDevice(
  _In_ TUsbDeviceData* pUsbDeviceData)
{
    CloseHandle(pUsbDeviceData->hDevice);

    pUsbDeviceData->hDevice = NULL;
}

static TUsbOverlappedData* lAllocOverlappedData(VOID)
{
    TUsbOverlappedData* pOverlappedData = (TUsbOverlappedData*)UtAllocMem(sizeof(TUsbOverlappedData));

    if (pOverlappedData == NULL)
    {
        return NULL;
    }

    pOverlappedData->Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (pOverlappedData->Overlapped.hEvent == NULL)
    {
        UtFreeMem(pOverlappedData);

        pOverlappedData = NULL;
    }

    return pOverlappedData;
}

static VOID lFreeOverlappedData(
  _In_ TUsbOverlappedData* pOverlappedData)
{
    if (pOverlappedData)
    {
        CloseHandle(pOverlappedData->Overlapped.hEvent);

        UtFreeMem(pOverlappedData);
    }
}

static VOID lZeroOverlapped(
  _In_ LPOVERLAPPED pOverlapped)
{
    HANDLE hEvent = pOverlapped->hEvent;

    ZeroMemory(pOverlapped, sizeof(OVERLAPPED));

    pOverlapped->hEvent = hEvent;
}

_Success_(return)
static BOOL lCancelOverlapped(
  _In_ HANDLE hDevice,
  _In_ LPOVERLAPPED pOverlapped,
  _In_ INT nTimeoutMs)
{
    DWORD dwError, dwNumberOfBytesTransferred;

    if (CancelIo(hDevice))
    {
        if (GetOverlappedResultEx(hDevice, pOverlapped, &dwNumberOfBytesTransferred,
                                  nTimeoutMs, TRUE))
        {
            return TRUE;
        }

        dwError = GetLastError();

        switch (dwError)
        {
            case ERROR_OPERATION_ABORTED:
                return TRUE;
            case ERROR_IO_INCOMPLETE:
                break;
        }
    }

    // Need to alloc new overlapped data and event

    return FALSE;
}

_Success_(return)
static BOOL lCancelFullSpeedDeviceOverlapped(
  _In_ LPOVERLAPPED pOverlapped,
  _In_ const TUsbFullSpeedDeviceData* pFullSpeedDeviceData,
  _In_ UCHAR PipeID,
  _In_ INT nTimeoutMs)
{
    DWORD dwError, dwNumberOfBytesTransferred;

    nTimeoutMs;

    if (pFullSpeedDeviceData->pAbortPipeFunc(pFullSpeedDeviceData->InterfaceHandle,
                                             PipeID))
    {
        if (pFullSpeedDeviceData->pGetOverlappedResultFunc(pFullSpeedDeviceData->InterfaceHandle,
                                                           pOverlapped,
                                                           &dwNumberOfBytesTransferred, 
                                                           TRUE))
        {
            return TRUE;
        }

        dwError = GetLastError();

        switch (dwError)
        {
            case ERROR_OPERATION_ABORTED:
                return TRUE;
            case ERROR_IO_INCOMPLETE:
                break;
        }
    }

    // Need to alloc new overlapped data and event

    return FALSE;
}

_Success_(return)
static BOOL lSendFullSpeedDeviceCommand(
  _Const_ _In_ const TUsbDeviceData* pDeviceData,
  _In_ INT nSendTimeoutMs,
  _In_ INT nReceiveTimeoutMs,
  _In_ INT nReceiveRetryCount,
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbWriteDataError,
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbReadDataError)
{
    const TUsbFullSpeedDeviceData* pFullSpeedDeviceData = &pDeviceData->DeviceData.UsbFullSpeedDeviceData;
    DWORD dwError;

    *pbWriteDataError = FALSE;
    *pbReadDataError = FALSE;

    lZeroOverlapped(&pDeviceData->pWriteOverlappedData->Overlapped);

    if (!ResetEvent(pDeviceData->pWriteOverlappedData->Overlapped.hEvent))
    {
        return FALSE;
    }

#if !defined(NDEBUG)
    lDumpCommandData(&pDeviceData->pWriteOverlappedData->Data.CommandData);
#endif

    Sleep(1);

    if (pFullSpeedDeviceData->pWritePipeFunc(pFullSpeedDeviceData->InterfaceHandle,
                                             CWritePipeID,
                                             (PUCHAR)&pDeviceData->pWriteOverlappedData->Data.CommandData,
                                             sizeof(TUtPepCommandData),
                                             NULL,
                                             &pDeviceData->pWriteOverlappedData->Overlapped))
    {
#if !defined(NDEBUG)
        lDumpMessage(L"Write pipe failed");
#endif

        return FALSE;
    }

    dwError = GetLastError();

    if (dwError != ERROR_IO_PENDING)
    {
#if !defined(NDEBUG)
        lDumpMessage(L"Write pipe did not return IO Pending error code");
#endif

        /*if (dwError == ERROR_BAD_COMMAND)
        {
            OutputDebugStringW(L"error bad command\r\n");
        }*/

        if (lCancelFullSpeedDeviceOverlapped(&pDeviceData->pWriteOverlappedData->Overlapped,
                                             pFullSpeedDeviceData,
                                             CWritePipeID,
                                             CDefCancelOverlappedTimeoutMs))
        {
        }
        else
        {
            *pbWriteDataError = TRUE;
        }

        return FALSE;
    }

    switch (WaitForSingleObject(pDeviceData->pWriteOverlappedData->Overlapped.hEvent,
                                nSendTimeoutMs))
    {
        case WAIT_OBJECT_0:
            // Success
            break;
        case WAIT_TIMEOUT:
#if !defined(NDEBUG)
            lDumpMessage(L"Write pipe timed out");
#endif

            if (lCancelFullSpeedDeviceOverlapped(&pDeviceData->pWriteOverlappedData->Overlapped,
                                                 pFullSpeedDeviceData,
                                                 CWritePipeID,
                                                 CDefCancelOverlappedTimeoutMs))
            {
            }
            else
            {
                *pbWriteDataError = TRUE;
            }

            return FALSE;
        case WAIT_ABANDONED:
            break;
        case WAIT_FAILED:
            dwError = GetLastError();
            break;
    }

    // Wait for the data to be received

    for (INT nRetryCount = 0; nRetryCount < nReceiveRetryCount; ++nRetryCount)
    {
        lZeroOverlapped(&pDeviceData->pReadOverlappedData->Overlapped);

        if (!ResetEvent(pDeviceData->pReadOverlappedData->Overlapped.hEvent))
        {
            return FALSE;
        }

        Sleep(1);

        if (pFullSpeedDeviceData->pReadPipeFunc(pFullSpeedDeviceData->InterfaceHandle,
                                                CReadPipeID,
                                                (PUCHAR)&pDeviceData->pReadOverlappedData->Data.ResponseData,
                                                sizeof(TUtPepResponseData),
                                                NULL,
                                                &pDeviceData->pReadOverlappedData->Overlapped))
        {
#if !defined(NDEBUG)
            lDumpMessage(L"Read pipe failed");
#endif

            return FALSE;
        }

        dwError = GetLastError();

        if (dwError != ERROR_IO_PENDING)
        {
            if (lCancelFullSpeedDeviceOverlapped(&pDeviceData->pReadOverlappedData->Overlapped,
                                                 pFullSpeedDeviceData,
                                                 CReadPipeID,
                                                 CDefCancelOverlappedTimeoutMs))
            {
            }
            else
            {
                *pbReadDataError = TRUE;
            }

            return FALSE;
        }

        switch (WaitForSingleObject(pDeviceData->pReadOverlappedData->Overlapped.hEvent,
                                    nReceiveTimeoutMs))
        {
            case WAIT_OBJECT_0:
                // Success

#if !defined(NDEBUG)
                lDumpResponseData(&pDeviceData->pReadOverlappedData->Data.ResponseData);
#endif

                if (pDeviceData->pReadOverlappedData->Data.ResponseData.ErrorCode != CPepErrorInitializeData)
                {
                    //return TRUE;
                }

                if (nRetryCount + 1 == nReceiveRetryCount)
                {
                    return TRUE;
                }
                break;
            case WAIT_TIMEOUT:
#if !defined(NDEBUG)
                lDumpMessage(L"Read pipe timed out");
#endif

                if (lCancelFullSpeedDeviceOverlapped(&pDeviceData->pReadOverlappedData->Overlapped,
                                                     pFullSpeedDeviceData,
                                                     CReadPipeID,
                                                     CDefCancelOverlappedTimeoutMs))
                {
                }
                else
                {
                    *pbReadDataError = TRUE;
                }
                break;
            case WAIT_ABANDONED:
                break;
            case WAIT_FAILED:
                dwError = GetLastError();
                break;
        }
    }

    return FALSE;
}

_Success_(return)
static BOOL lSendHidDeviceCommand(
  _Const_ _In_ const TUsbDeviceData* pDeviceData,
  _In_ INT nSendTimeoutMs,
  _In_ INT nReceiveTimeoutMs,
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbWriteDataError,
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbReadDataError)
{
    const TUtPepCommandData* pCommandData = &pDeviceData->pWriteOverlappedData->Data.CommandData;
    TUtPepResponseData* pResponseData = &pDeviceData->pReadOverlappedData->Data.ResponseData;
    INT nWriteDataLen = sizeof(TUtPepCommandData) + 1;
    INT nReadDataLen = sizeof(TUtPepResponseData) + 1;
    DWORD dwBytesRead, dwBytesWritten, dwError;
    LPBYTE pbyWriteData, pbyReadData;

    *pbWriteDataError = FALSE;
    *pbReadDataError = FALSE;

    pbyWriteData = (LPBYTE)UtAllocMem(nWriteDataLen);

    if (pbyWriteData == NULL)
    {
        return FALSE;
    }

    pbyReadData = (LPBYTE)UtAllocMem(nReadDataLen);

    if (pbyReadData == NULL)
    {
        UtFreeMem(pbyWriteData);

        return FALSE;
    }

    lZeroOverlapped(&pDeviceData->pReadOverlappedData->Overlapped);
    lZeroOverlapped(&pDeviceData->pWriteOverlappedData->Overlapped);

    if (!ResetEvent(pDeviceData->pReadOverlappedData->Overlapped.hEvent) ||
        !ResetEvent(pDeviceData->pWriteOverlappedData->Overlapped.hEvent))
    {
        UtFreeMem(pbyWriteData);
        UtFreeMem(pbyReadData);

        return FALSE;
    }

    *pbyReadData = 0;
    *pbyWriteData = 0;

    CopyMemory(pbyWriteData + 1, pCommandData, sizeof(TUtPepCommandData));

    if (ReadFile(pDeviceData->hDevice, pbyReadData, nReadDataLen,
                 &dwBytesRead, &pDeviceData->pReadOverlappedData->Overlapped))
    {
        UtFreeMem(pbyWriteData);
        UtFreeMem(pbyReadData);

        return FALSE;
    }

    dwError = GetLastError();

    if (dwError != ERROR_IO_PENDING)
    {
        UtFreeMem(pbyWriteData);
        UtFreeMem(pbyReadData);

        return FALSE;
    }

    if (!WriteFile(pDeviceData->hDevice, pbyWriteData, nWriteDataLen,
                   &dwBytesWritten, &pDeviceData->pWriteOverlappedData->Overlapped))
    {
        dwError = GetLastError();

        if (dwError != ERROR_IO_PENDING)
        {
            if (lCancelOverlapped(pDeviceData->hDevice, &pDeviceData->pReadOverlappedData->Overlapped,
                                  CDefCancelOverlappedTimeoutMs))
            {
                UtFreeMem(pbyWriteData);
                UtFreeMem(pbyReadData);
            }
            else
            {
                *pbReadDataError = TRUE;

                UtFreeMem(pbyWriteData);
            }

            return FALSE;
        }

        switch (WaitForSingleObject(pDeviceData->pWriteOverlappedData->Overlapped.hEvent,
                                    nSendTimeoutMs))
        {
            case WAIT_OBJECT_0:
                // Success
                break;
            case WAIT_TIMEOUT:
                if (lCancelOverlapped(pDeviceData->hDevice,
                                      &pDeviceData->pWriteOverlappedData->Overlapped,
                                      CDefCancelOverlappedTimeoutMs))
                {
                    UtFreeMem(pbyWriteData);
                }
                else
                {
                    *pbWriteDataError = TRUE;
                }

                if (lCancelOverlapped(pDeviceData->hDevice,
                                      &pDeviceData->pReadOverlappedData->Overlapped,
                                      CDefCancelOverlappedTimeoutMs))
                {
                    UtFreeMem(pbyReadData);
                }
                else
                {
                    *pbReadDataError = TRUE;
                }

                return FALSE;
            case WAIT_ABANDONED:
                break;
            case WAIT_FAILED:
                dwError = GetLastError();
                break;
        }
    }

    // Wait for the data to be received

    switch (WaitForSingleObject(pDeviceData->pReadOverlappedData->Overlapped.hEvent, 
                                nReceiveTimeoutMs))
    {
        case WAIT_OBJECT_0:
            // Success
            CopyMemory(pResponseData, pbyReadData + 1, sizeof(TUtPepResponseData));

            UtFreeMem(pbyWriteData);
            UtFreeMem(pbyReadData);

            return TRUE;
        case WAIT_TIMEOUT:
            if (lCancelOverlapped(pDeviceData->hDevice,
                                  &pDeviceData->pReadOverlappedData->Overlapped,
                                  CDefCancelOverlappedTimeoutMs))
            { 
                UtFreeMem(pbyWriteData);
                UtFreeMem(pbyReadData);
            }
            else
            {
                *pbReadDataError = TRUE;

                UtFreeMem(pbyWriteData);
            }
            break;
        case WAIT_ABANDONED:
            break;
        case WAIT_FAILED:
            dwError = GetLastError();
            break;
    }

    return FALSE;
}

_Success_(return)
static BOOL lSendDeviceCommand(
  _In_ TUsbDeviceData* pDeviceData,
  _In_ INT nSendTimeoutMs,
  _In_ INT nReceiveTimeoutMs)
{
    BOOL bResult = FALSE;
    BOOL bWriteDataError = FALSE;
    BOOL bReadDataError = FALSE;
#if !defined(ENABLE_TIMING)
    SYSTEMTIME StartTime, EndTime;
    FILETIME StartFileTime, EndFileTime;
    ULARGE_INTEGER StartInteger, EndInteger, ElapsedInteger;
#endif

    ZeroMemory(&pDeviceData->pReadOverlappedData->Data.ResponseData,
               sizeof(pDeviceData->pReadOverlappedData->Data.ResponseData));

#if !defined(ENABLE_TIMING)
    GetSystemTime(&StartTime);
#endif

    switch (lFindUsbDeviceType(&pDeviceData->DeviceInterfaceGuid))
    {
        case eUsbHIDDeviceType:
            bResult = lSendHidDeviceCommand(pDeviceData, nSendTimeoutMs, nReceiveTimeoutMs,
                                            &bWriteDataError, &bReadDataError);
            break;
        case eUsbFullSpeedDeviceType:
            bResult = lSendFullSpeedDeviceCommand(pDeviceData, nSendTimeoutMs, nReceiveTimeoutMs,
                                                  CReceiveRetryCount,
                                                  &bWriteDataError, &bReadDataError);
            break;
    }

#if !defined(ENABLE_TIMING)
    GetSystemTime(&EndTime);

    SystemTimeToFileTime(&StartTime, &StartFileTime);
    SystemTimeToFileTime(&EndTime, &EndFileTime);

    StartInteger.LowPart = StartFileTime.dwLowDateTime;
    StartInteger.HighPart = StartFileTime.dwHighDateTime;

    EndInteger.LowPart = EndFileTime.dwLowDateTime;
    EndInteger.HighPart = EndFileTime.dwHighDateTime;

    ElapsedInteger.QuadPart = EndInteger.QuadPart - StartInteger.QuadPart;
#endif

    if (bReadDataError)
    {
        // I/O operation did not complete so have no choice but to leak memory
        // and the event handle.

        pDeviceData->pReadOverlappedData = lAllocOverlappedData();
    }

    if (bWriteDataError)
    {
        // I/O operation did not complete so have no choice but to leak memory
        // and the event handle.

        pDeviceData->pWriteOverlappedData = lAllocOverlappedData();
    }

    return bResult;
}

static BOOL lGetDeviceInputReportLength(
  _In_ HANDLE hDevice,
  _Out_writes_(sizeof(INT)) LPINT pnInputReportLen)
{
    BOOL bResult = FALSE;
    HMODULE hModule = LoadLibraryW(CHidLibrary);
    TGetPreparsedDataFunc pGetPreparsedDataFunc;
    TGetCapsFunc pGetCapsFunc;
    TFreePreparsedDataFunc pFreePreparsedDataFunc;
    PHIDP_PREPARSED_DATA pPreparsedData;
    HIDP_CAPS Caps;

    *pnInputReportLen = 0;

    if (hModule == NULL)
    {
        return FALSE;
    }

    pGetPreparsedDataFunc = (TGetPreparsedDataFunc)GetProcAddress(hModule, "HidD_GetPreparsedData");
    pGetCapsFunc = (TGetCapsFunc)GetProcAddress(hModule, "HidP_GetCaps");
    pFreePreparsedDataFunc = (TFreePreparsedDataFunc)GetProcAddress(hModule, "HidD_FreePreparsedData");

    if (pGetPreparsedDataFunc == NULL || pGetCapsFunc == NULL || pFreePreparsedDataFunc == NULL)
    {
        FreeLibrary(hModule);

        return FALSE;
    }

    if (!pGetPreparsedDataFunc(hDevice, &pPreparsedData))
    {
        FreeLibrary(hModule);

        return FALSE;
    }

    if (HIDP_STATUS_SUCCESS == pGetCapsFunc(pPreparsedData, &Caps))
    {
        *pnInputReportLen = Caps.InputReportByteLength;

        bResult = TRUE;
    }

    pFreePreparsedDataFunc(pPreparsedData);

    FreeLibrary(hModule);

    return bResult;
}

static BOOL lGetUsbVersion()
{
    BOOL bResult = FALSE;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    pCommandData->Command = CPepVersionCommand;

    if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                           CDefReceiveTimeoutMs))
    {
        if (pResponseData->ErrorCode == CPepErrorSuccess)
        {
            //OutputDebugStringW((LPWSTR)&pResponseData->Data.Version);
            //OutputDebugStringW(L"\r\n");

            bResult = TRUE;
        }
    }

    return bResult;
}

static BOOL lRefreshDeviceInterfaces(
  _In_ HWND hWnd)
{
    HMODULE hModule = LoadLibraryW(CSetupApiLibrary);
    TSetupDiGetClassDevsFunc pSetupDiGetClassDevsFunc;
    TSetupDiEnumDeviceInterfacesFunc pSetupDiEnumDeviceInterfacesFunc;
    TSetupDiGetDeviceInterfaceDetailFunc pSetupDiGetDeviceInterfaceDetailFunc;
    TSetupDiDestroyDeviceInfoListFunc pSetupDiDestroyDeviceInfoListFunc;
    HDEVINFO hDevInfo;
    DWORD dwDevCount, dwBufferLen, dwRequiredLen;
    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData;
    PDEV_BROADCAST_DEVICEINTERFACE pDevBroadcastDeviceInterface;
    INT nDevBroadcastDeviceInterfaceLen, nDevicePathLen;

    if (hModule == NULL)
    {
        return FALSE;
    }

    pSetupDiGetClassDevsFunc = (TSetupDiGetClassDevsFunc)GetProcAddress(hModule, "SetupDiGetClassDevsW");
    pSetupDiEnumDeviceInterfacesFunc = (TSetupDiEnumDeviceInterfacesFunc)GetProcAddress(hModule, "SetupDiEnumDeviceInterfaces");
    pSetupDiGetDeviceInterfaceDetailFunc = (TSetupDiGetDeviceInterfaceDetailFunc)GetProcAddress(hModule, "SetupDiGetDeviceInterfaceDetailW");
    pSetupDiDestroyDeviceInfoListFunc = (TSetupDiDestroyDeviceInfoListFunc)GetProcAddress(hModule, "SetupDiDestroyDeviceInfoList");

    if (pSetupDiGetClassDevsFunc == NULL ||
        pSetupDiEnumDeviceInterfacesFunc == NULL ||
        pSetupDiGetDeviceInterfaceDetailFunc == NULL ||
        pSetupDiDestroyDeviceInfoListFunc == NULL)
    {
        FreeLibrary(hModule);

        return FALSE;
    }

    for (INT nIndex = 0; nIndex < MArrayLen(l_UsbGuidTypeData); ++nIndex)
    {
        if (l_UsbGuidTypeData[nIndex].UsbDeviceType == eUsbInvalidDeviceType)
        {
            continue;
        }

        hDevInfo = pSetupDiGetClassDevsFunc(&l_UsbGuidTypeData[nIndex].Guid, NULL, NULL,
                                            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

        if (hDevInfo == INVALID_HANDLE_VALUE)
        {
            continue;
        }

        dwDevCount = 0;

        DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);

        while (pSetupDiEnumDeviceInterfacesFunc(hDevInfo, 0, &l_UsbGuidTypeData[nIndex].Guid,
                                                dwDevCount, &DeviceInterfaceData))
        {
            dwBufferLen = 0;

            pSetupDiGetDeviceInterfaceDetailFunc(hDevInfo, &DeviceInterfaceData, 0, 0,
                                                 &dwBufferLen, 0);

            pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)UtAllocMem(dwBufferLen);

            if (pDeviceInterfaceDetailData == NULL)
            {
                ++dwDevCount;

                continue;
            }

            pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            if (!pSetupDiGetDeviceInterfaceDetailFunc(hDevInfo, &DeviceInterfaceData,
                                                      pDeviceInterfaceDetailData,
                                                      dwBufferLen, &dwRequiredLen, NULL))
            {
                UtFreeMem(pDeviceInterfaceDetailData);

                ++dwDevCount;

                continue;
            }

            nDevicePathLen = lstrlenW(pDeviceInterfaceDetailData->DevicePath) + 1;
            nDevBroadcastDeviceInterfaceLen = sizeof(DEV_BROADCAST_DEVICEINTERFACE) +
                                                  (nDevicePathLen * sizeof(WCHAR));

            pDevBroadcastDeviceInterface = UtAllocMem(nDevBroadcastDeviceInterfaceLen);

            if (pDevBroadcastDeviceInterface)
            {
                pDevBroadcastDeviceInterface->dbcc_size = sizeof(nDevBroadcastDeviceInterfaceLen);
                pDevBroadcastDeviceInterface->dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
                pDevBroadcastDeviceInterface->dbcc_reserved = 0;
                pDevBroadcastDeviceInterface->dbcc_classguid = l_UsbGuidTypeData[nIndex].Guid;

                StringCchCopyW(pDevBroadcastDeviceInterface->dbcc_name, nDevicePathLen,
                               pDeviceInterfaceDetailData->DevicePath);

                SendMessage(hWnd, WM_DEVICECHANGE, DBT_DEVICEARRIVAL,
                            (LPARAM)pDevBroadcastDeviceInterface);

                UtFreeMem(pDeviceInterfaceDetailData);
            }

            ++dwDevCount;
        }

        pSetupDiDestroyDeviceInfoListFunc(hDevInfo);
    }

    FreeLibrary(hModule);

    return TRUE;
}

static BOOL lRegisterDeviceInterfaceNotifications(
  _In_ HWND hWnd,
  _Out_ HDEVNOTIFY** ppDevNotify)
{
    INT nDevNotifyLen = 0;
    DEV_BROADCAST_DEVICEINTERFACE DevBroadcastDeviceInterface;
    HDEVNOTIFY hDevNotify;

    *ppDevNotify = (HDEVNOTIFY*)UtAllocMem(sizeof(HDEVNOTIFY) * (MArrayLen(l_UsbGuidTypeData) + 1));

    if (*ppDevNotify == NULL)
    {
        return FALSE;
    }

    for (INT nIndex = 0; nIndex < MArrayLen(l_UsbGuidTypeData); ++nIndex)
    {
        ZeroMemory(&DevBroadcastDeviceInterface, sizeof(DevBroadcastDeviceInterface));

        DevBroadcastDeviceInterface.dbcc_size = sizeof(DevBroadcastDeviceInterface);
        DevBroadcastDeviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

        if (l_UsbGuidTypeData[nIndex].UsbDeviceType != eUsbInvalidDeviceType)
        {
            CopyMemory(&DevBroadcastDeviceInterface.dbcc_classguid,
                       &l_UsbGuidTypeData[nIndex].Guid,
                       sizeof(GUID));

            hDevNotify = RegisterDeviceNotification(hWnd, &DevBroadcastDeviceInterface,
                                                    DEVICE_NOTIFY_WINDOW_HANDLE);

            if (hDevNotify)
            {
                *(*ppDevNotify + nDevNotifyLen) = hDevNotify;

                ++nDevNotifyLen;
            }
        }
    }

    *(*ppDevNotify + nDevNotifyLen) = NULL;

    return TRUE;
}

static VOID lUnregisterDeviceInterfaceNotifications(
  _In_ HDEVNOTIFY* pDevNotify)
{
    INT nIndex = 0;

    while (pDevNotify[nIndex])
    {
        UnregisterDeviceNotification(pDevNotify[nIndex]);

        ++nIndex;
    }

    UtFreeMem(pDevNotify);
}

static VOID lDeviceArrivalBroadcastDeviceInterface(
  _In_ HWND hWnd,
  _In_ PDEV_BROADCAST_DEVICEINTERFACE pDevBroadcastDeviceInterface)
{
    TUsbDeviceData* pUsbDeviceData = (TUsbDeviceData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    HANDLE hDevice = NULL;
    INT nDeviceNameLen;

    WaitForSingleObject(*(pUsbDeviceData->phDeviceMutex), INFINITE);

    if (pUsbDeviceData->hDevice)
    {
        ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

        return;
    }

    switch (lFindUsbDeviceType(&pDevBroadcastDeviceInterface->dbcc_classguid))
    {
        case eUsbHIDDeviceType:
            hDevice = lOpenHidDevice(pDevBroadcastDeviceInterface->dbcc_name);

            if (hDevice == NULL)
            {
                ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

                return;
            }

            if (!lGetDeviceInputReportLength(hDevice, &pUsbDeviceData->DeviceData.UsbHidDeviceData.nInputReportLen))
            {
                ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

                CloseHandle(hDevice);

                return;
            }
            break;
        case eUsbFullSpeedDeviceType:
            hDevice = lOpenFullSpeedDevice(pDevBroadcastDeviceInterface->dbcc_name);

            if (hDevice == NULL)
            {
                ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

                return;
            }

            if (!lInitializeUsbFullSpeedDeviceData(&pUsbDeviceData->DeviceData.UsbFullSpeedDeviceData))
            {
                ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

                CloseHandle(hDevice);

                return;
            }

            if (!pUsbDeviceData->DeviceData.UsbFullSpeedDeviceData.pInitializeFunc(hDevice,
                    &pUsbDeviceData->DeviceData.UsbFullSpeedDeviceData.InterfaceHandle))
            {
                lUninitializeUsbFullSpeedDeviceData(&pUsbDeviceData->DeviceData.UsbFullSpeedDeviceData);

                ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

                CloseHandle(hDevice);

                return;
            }
            break;
    }

    if (hDevice == NULL)
    {
        ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

        return;
    }

    pUsbDeviceData->DeviceInterfaceGuid = pDevBroadcastDeviceInterface->dbcc_classguid;

    nDeviceNameLen = lstrlenW(pDevBroadcastDeviceInterface->dbcc_name) + 1;

    pUsbDeviceData->pszDeviceName = (LPWSTR)UtAllocMem(nDeviceNameLen * sizeof(WCHAR));

    if (pUsbDeviceData->pszDeviceName == NULL)
    {
        ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

        CloseHandle(hDevice);

        return;
    }

    StringCchCopyW(pUsbDeviceData->pszDeviceName, nDeviceNameLen, pDevBroadcastDeviceInterface->dbcc_name);

    pUsbDeviceData->pReadOverlappedData = lAllocOverlappedData();

    if (pUsbDeviceData->pReadOverlappedData == NULL)
    {
        ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

        CloseHandle(hDevice);

        UtFreeMem(pUsbDeviceData->pszDeviceName);

        return;
    }

    pUsbDeviceData->pWriteOverlappedData = lAllocOverlappedData();

    if (pUsbDeviceData->pWriteOverlappedData == NULL)
    {
        ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

        CloseHandle(hDevice);

        UtFreeMem(pUsbDeviceData->pszDeviceName);

        lFreeOverlappedData(pUsbDeviceData->pReadOverlappedData);

        return;
    }

    pUsbDeviceData->hDevice = hDevice;

    // Need to send a command to the device or else the initial command
    // will not be executed for some odd reason.

    lGetUsbVersion();

    lPushPepCtrlDeviceChange(*l_DeviceData.UsbDeviceData.phDeviceChangeListMutex,
                             l_DeviceData.UsbDeviceData.ppPepCtrlDeviceChange,
                             l_DeviceData.UsbDeviceData.pnPepCtrlDeviceChangeLen,
                             eUtPepCtrlDeviceArrived);

    ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));
}

static VOID lDeviceRemovalBroadcastDeviceInterface(
  _In_ HWND hWnd,
  _In_ PDEV_BROADCAST_DEVICEINTERFACE pDevBroadcastDeviceInterface)
{
    TUsbDeviceData* pUsbDeviceData = (TUsbDeviceData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    WaitForSingleObject(*(pUsbDeviceData->phDeviceMutex), INFINITE);

    if (pUsbDeviceData->hDevice &&
        IsEqualGUID(&pDevBroadcastDeviceInterface->dbcc_classguid, &pUsbDeviceData->DeviceInterfaceGuid) &&
        lstrcmpiW(pDevBroadcastDeviceInterface->dbcc_name, pUsbDeviceData->pszDeviceName) == 0)
    {
        switch (lFindUsbDeviceType(&pUsbDeviceData->DeviceInterfaceGuid))
        {
            case eUsbHIDDeviceType:
#if !defined(NDEBUG)
                pUsbDeviceData->DeviceData.UsbHidDeviceData.nInputReportLen = 0;
#endif
                break;
            case eUsbFullSpeedDeviceType:
                if (!pUsbDeviceData->DeviceData.UsbFullSpeedDeviceData.pFreeFunc(
                        pUsbDeviceData->DeviceData.UsbFullSpeedDeviceData.InterfaceHandle))
                {
                    // error occurred
                }

                lUninitializeUsbFullSpeedDeviceData(&pUsbDeviceData->DeviceData.UsbFullSpeedDeviceData);
                break;
        }

#if !defined(NDEBUG)
        ZeroMemory(&pUsbDeviceData->DeviceInterfaceGuid, sizeof(pUsbDeviceData->DeviceInterfaceGuid));
#endif

        lCloseDevice(pUsbDeviceData);

        UtFreeMem(pUsbDeviceData->pszDeviceName);

        lFreeOverlappedData(pUsbDeviceData->pReadOverlappedData);
        lFreeOverlappedData(pUsbDeviceData->pWriteOverlappedData);

        pUsbDeviceData->pszDeviceName = NULL;

        lPushPepCtrlDeviceChange(*pUsbDeviceData->phDeviceChangeListMutex,
                                 pUsbDeviceData->ppPepCtrlDeviceChange,
                                 pUsbDeviceData->pnPepCtrlDeviceChangeLen,
                                 eUtPepCtrlDeviceRemoved);
    }

    ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));
}

static VOID lDeviceArrival(
  _In_ HWND hWnd,
  _In_ PDEV_BROADCAST_HDR pDevBroadcastHdr)
{
    switch (pDevBroadcastHdr->dbch_devicetype)
    {
        case DBT_DEVTYP_DEVICEINTERFACE:
            lDeviceArrivalBroadcastDeviceInterface(hWnd, (PDEV_BROADCAST_DEVICEINTERFACE)pDevBroadcastHdr);
            break;
    }
}

static VOID lDeviceRemoval(
  _In_ HWND hWnd,
  _In_ PDEV_BROADCAST_HDR pDevBroadcastHdr)
{
    switch (pDevBroadcastHdr->dbch_devicetype)
    {
        case DBT_DEVTYP_DEVICEINTERFACE:
            lDeviceRemovalBroadcastDeviceInterface(hWnd, (PDEV_BROADCAST_DEVICEINTERFACE)pDevBroadcastHdr);
            break;
    }
}

static LRESULT lOnDeviceChange(
  _In_ HWND hWnd,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam)
{
    switch (wParam)
    {
        case DBT_DEVICEARRIVAL:
            lDeviceArrival(hWnd, (PDEV_BROADCAST_HDR)lParam);
            break;
        case DBT_DEVICEREMOVECOMPLETE:
            lDeviceRemoval(hWnd, (PDEV_BROADCAST_HDR)lParam);
            break;
    }

    return TRUE;
}

static LRESULT CALLBACK lDeviceNotifyCtrlWndProc(
  _In_ HWND hWnd,
  _In_ UINT uMsg,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DEVICECHANGE:
            return lOnDeviceChange(hWnd, wParam, lParam);
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

VOID lDeviceNotifyCtrlRegister(VOID)
{
    WNDCLASSEXW WndClassEx;

    WndClassEx.cbSize = sizeof(WndClassEx);
    WndClassEx.style = CS_GLOBALCLASS;
    WndClassEx.lpfnWndProc = lDeviceNotifyCtrlWndProc;
    WndClassEx.cbClsExtra = 0;
    WndClassEx.cbWndExtra = sizeof(TUsbDeviceData*);
    WndClassEx.hInstance = UtPepCtrlGetInstance();
    WndClassEx.hIcon = NULL;
    WndClassEx.hCursor = NULL;
    WndClassEx.hbrBackground = NULL;
    WndClassEx.lpszMenuName = NULL;
    WndClassEx.lpszClassName = CUiDeviceNotificationCtrlClass;
    WndClassEx.hIconSm = NULL;

    RegisterClassExW(&WndClassEx);
}

VOID lDeviceNotifyCtrlUnregister(VOID)
{
    UnregisterClassW(CUiDeviceNotificationCtrlClass, UtPepCtrlGetInstance());
}

static DWORD WINAPI lDeviceChangeThreadProc(
  _In_ LPVOID pvParameter)
{
    TDeviceChangeData* pDeviceChangeData = (TDeviceChangeData*)pvParameter;
    BOOL bQuit = FALSE;
    HWND hWnd;
    MSG Msg;
    HDEVNOTIFY* phDevNotify;

    lDeviceNotifyCtrlRegister();

    hWnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW, CUiDeviceNotificationCtrlClass,
                           NULL, WS_OVERLAPPED, 0, 0, 0, 0, GetDesktopWindow(),
                           NULL, UtPepCtrlGetInstance(), NULL);

    if (hWnd == NULL)
    {
        pDeviceChangeData->bThreadInitialized = FALSE;

        SetEvent(pDeviceChangeData->hThreadInitializedEvent);

        lDeviceNotifyCtrlUnregister();

        return 0;
    }

    if (!lRegisterDeviceInterfaceNotifications(hWnd, &phDevNotify))
    {
        DestroyWindow(hWnd);

        pDeviceChangeData->bThreadInitialized = FALSE;

        SetEvent(pDeviceChangeData->hThreadInitializedEvent);

        lDeviceNotifyCtrlUnregister();

        return 0;
    }

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pDeviceChangeData->pUsbDeviceData);

    pDeviceChangeData->bThreadInitialized = TRUE;

    lRefreshDeviceInterfaces(hWnd);

    SetEvent(pDeviceChangeData->hThreadInitializedEvent);

    while (!bQuit)
    {
        switch (MsgWaitForMultipleObjects(1, &pDeviceChangeData->hQuitEvent, FALSE,
                                          INFINITE, QS_ALLINPUT))
        {
            case WAIT_OBJECT_0:
                bQuit = TRUE;
                break;
            case WAIT_OBJECT_0 + 1:
                if (PeekMessage(&Msg, hWnd, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Msg);
                    DispatchMessage(&Msg);
                }
                break;
        }
    }

    lUnregisterDeviceInterfaceNotifications(phDevNotify);

    DestroyWindow(hWnd);

    lDeviceNotifyCtrlUnregister();

    return 0;
}

#pragma endregion

BOOL UsbPepCtrlInitialize(
  _In_ TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc)
{
    DWORD dwThreadId;

    lInitializeGuidTypeData();

    // Dispatch Device Change Data

    l_DeviceData.DispatchDeviceChangeData.pDeviceChangeFunc = pDeviceChangeFunc;
    l_DeviceData.DispatchDeviceChangeData.hQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    l_DeviceData.DispatchDeviceChangeData.nPepCtrlDeviceChangeLen = 0;

    if (l_DeviceData.DispatchDeviceChangeData.hQuitEvent == NULL)
    {
        return FALSE;
    }

    l_DeviceData.DispatchDeviceChangeData.hDeviceChangeListMutex = CreateMutex(NULL, FALSE, NULL);

    if (l_DeviceData.DispatchDeviceChangeData.hDeviceChangeListMutex == NULL)
    {
        lCloseAll();

        return FALSE;
    }

    l_DeviceData.DispatchDeviceChangeData.pPepCtrlDeviceChange = (EUtPepCtrlDeviceChange*)UtAllocMem(0);

    if (l_DeviceData.DispatchDeviceChangeData.pPepCtrlDeviceChange == NULL)
    {
        lCloseAll();

        return FALSE;
    }

    l_DeviceData.hDispatchDeviceChangeThread = CreateThread(NULL, 0, lDispatchDeviceChangeThreadProc,
                                                            &l_DeviceData.DispatchDeviceChangeData,
                                                            0, &dwThreadId);

    if (l_DeviceData.hDispatchDeviceChangeThread == NULL)
    {
        lCloseAll();

        return FALSE;
    }

    // USB Device Data

    l_DeviceData.UsbDeviceData.phDeviceChangeListMutex = &l_DeviceData.DispatchDeviceChangeData.hDeviceChangeListMutex;
    l_DeviceData.UsbDeviceData.ppPepCtrlDeviceChange = &l_DeviceData.DispatchDeviceChangeData.pPepCtrlDeviceChange;
    l_DeviceData.UsbDeviceData.pnPepCtrlDeviceChangeLen = &l_DeviceData.DispatchDeviceChangeData.nPepCtrlDeviceChangeLen;

    // Device Change Data

    l_DeviceChangeData.bThreadInitialized = FALSE;
    l_DeviceChangeData.pUsbDeviceData = &l_DeviceData.UsbDeviceData;
    l_DeviceChangeData.pDispatchDeviceChangeData = &l_DeviceData.DispatchDeviceChangeData;
    l_DeviceChangeData.hQuitEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    if (l_DeviceChangeData.hQuitEvent == NULL)
    {
        lCloseAll();

        return FALSE;
    }

    l_DeviceChangeData.hThreadInitializedEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    if (l_DeviceChangeData.hThreadInitializedEvent == NULL)
    {
        lCloseAll();

        return FALSE;
    }

    l_DeviceData.hDeviceMutex = CreateMutex(NULL, FALSE, NULL);

    if (l_DeviceData.hDeviceMutex == NULL)
    {
        lCloseAll();

        return FALSE;
    }

    l_DeviceData.UsbDeviceData.phDeviceMutex = &l_DeviceData.hDeviceMutex;

    l_DeviceData.hDeviceChangeThread = CreateThread(NULL, 0, lDeviceChangeThreadProc,
                                                    &l_DeviceChangeData,
                                                    0, &dwThreadId);

    if (l_DeviceData.hDeviceChangeThread == NULL)
    {
        lCloseAll();

        return FALSE;
    }

    WaitForSingleObject(l_DeviceChangeData.hThreadInitializedEvent, INFINITE);

    CloseHandle(l_DeviceChangeData.hThreadInitializedEvent);

    l_DeviceChangeData.hThreadInitializedEvent = NULL;

    if (l_DeviceChangeData.bThreadInitialized == FALSE)
    {
        lCloseAll();

        return FALSE;
    }

    return TRUE;
}

BOOL UsbPepCtrlUninitialize(VOID)
{
    lCloseAll();

    return TRUE;
}

_Success_(return)
BOOL UsbPepCtrlGetDeviceName(
  _Out_ LPWSTR pszDeviceName,
  _Out_ LPINT pnDeviceNameLen)
{
    BOOL bResult = FALSE;
    INT nDeviceNameLen = 1;

    if (pszDeviceName == NULL && pnDeviceNameLen == NULL)
    {
        return FALSE;
    }

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pszDeviceName)
    {
        nDeviceNameLen = lstrlenW(l_DeviceData.UsbDeviceData.pszDeviceName) + 1;
    }

    if (pszDeviceName == NULL)
    {
        *pnDeviceNameLen = nDeviceNameLen;

        bResult = TRUE;
    }
    else
    {
        if (pnDeviceNameLen && *pnDeviceNameLen >= nDeviceNameLen)
        {
            if (l_DeviceData.UsbDeviceData.pszDeviceName)
            {
                StringCchCopy(pszDeviceName, *pnDeviceNameLen, l_DeviceData.UsbDeviceData.pszDeviceName);
            }
            else
            {
                *pszDeviceName = 0;
            }

            bResult = TRUE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

BOOL UsbPepCtrlSetDeviceName(
  _Const_ _In_ LPCWSTR pszDeviceName)
{
    pszDeviceName;

    return FALSE;
}

BOOL UsbPepCtrlSetDelaySettings(
  _In_ UINT32 nChipEnableNanoSeconds,
  _In_ UINT32 nOutputEnableNanoSeconds)
{
    BOOL bResult = FALSE;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pWriteOverlappedData == NULL ||
        l_DeviceData.UsbDeviceData.pReadOverlappedData == NULL)
    {
        ReleaseMutex(l_DeviceData.hDeviceMutex);

        return FALSE;
    }

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    lIncrementPacketID(pCommandData);

    pCommandData->Command = CPepSetDelaysCommand;
    pCommandData->Data.Delays.nChipEnableNanoSeconds = nChipEnableNanoSeconds;
    pCommandData->Data.Delays.nOutputEnableNanoSeconds = nOutputEnableNanoSeconds;

    if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                           CDefReceiveTimeoutMs))
    {
        if (pResponseData->PacketID == pCommandData->PacketID)
        {
            if (pResponseData->ErrorCode == CPepErrorSuccess)
            {
                bResult = TRUE;
            }
        }
        else
        {
#if !defined(NDEBUG)
            lDumpNonMatchingPacketID(pCommandData, pResponseData);
#endif

            bResult = FALSE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

_Success_(return)
BOOL UsbPepCtrlIsDevicePresent(
  _Out_writes_(sizeof(BOOL)) LPBOOL pbPresent)
{
    *pbPresent = l_DeviceData.UsbDeviceData.hDevice ? TRUE : FALSE;

    return TRUE;
}

BOOL UsbPepCtrlReset(VOID)
{
    BOOL bResult = FALSE;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pWriteOverlappedData == NULL ||
        l_DeviceData.UsbDeviceData.pReadOverlappedData == NULL)
    {
        ReleaseMutex(l_DeviceData.hDeviceMutex);

        return FALSE;
    }

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    lIncrementPacketID(pCommandData);

    pCommandData->Command = CPepResetCommand;

    if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                           CDefReceiveTimeoutMs))
    {
        if (pResponseData->PacketID == pCommandData->PacketID)
        {
            if (pResponseData->ErrorCode == CPepErrorSuccess)
            {
                bResult = TRUE;
            }
        }
        else
        {
#if !defined(NDEBUG)
            lDumpNonMatchingPacketID(pCommandData, pResponseData);
#endif

            bResult = FALSE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

BOOL UsbPepCtrlSetProgrammerMode(
  _In_ UINT32 nProgrammerMode)
{
    BOOL bResult = FALSE;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pWriteOverlappedData == NULL ||
        l_DeviceData.UsbDeviceData.pReadOverlappedData == NULL)
    {
        ReleaseMutex(l_DeviceData.hDeviceMutex);

        return FALSE;
    }

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    lIncrementPacketID(pCommandData);

    pCommandData->Command = CPepSetProgrammerModeCommand;
    pCommandData->Data.nProgrammerMode = nProgrammerMode;

    if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                           CDefReceiveTimeoutMs))
    {
        if (pResponseData->PacketID == pCommandData->PacketID)
        {
            if (pResponseData->ErrorCode == CPepErrorSuccess)
            {
                bResult = TRUE;
            }
        }
        else
        {
#if !defined(NDEBUG)
            lDumpNonMatchingPacketID(pCommandData, pResponseData);
#endif

            bResult = FALSE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

BOOL UsbPepCtrlSetVccMode(
  _In_ UINT32 nVccMode)
{
    BOOL bResult = FALSE;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pWriteOverlappedData == NULL ||
        l_DeviceData.UsbDeviceData.pReadOverlappedData == NULL)
    {
        ReleaseMutex(l_DeviceData.hDeviceMutex);

        return FALSE;
    }

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    lIncrementPacketID(pCommandData);

    pCommandData->Command = CPepSetVccModeCommand;
    pCommandData->Data.nVccMode = nVccMode;

    if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                           CDefReceiveTimeoutMs))
    {
        if (pResponseData->PacketID == pCommandData->PacketID)
        {
            if (pResponseData->ErrorCode == CPepErrorSuccess)
            {
                bResult = TRUE;
            }
        }
        else
        {
#if !defined(NDEBUG)
            lDumpNonMatchingPacketID(pCommandData, pResponseData);
#endif

            bResult = FALSE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

BOOL UsbPepCtrlSetPinPulseMode(
  _In_ UINT32 nPinPulseMode)
{
    BOOL bResult = FALSE;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pWriteOverlappedData == NULL ||
        l_DeviceData.UsbDeviceData.pReadOverlappedData == NULL)
    {
        ReleaseMutex(l_DeviceData.hDeviceMutex);

        return FALSE;
    }

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    lIncrementPacketID(pCommandData);

    pCommandData->Command = CPepSetPinPulseModeCommand;
    pCommandData->Data.nPinPulseMode = nPinPulseMode;

    if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                           CDefReceiveTimeoutMs))
    {
        if (pResponseData->PacketID == pCommandData->PacketID)
        {
            if (pResponseData->ErrorCode == CPepErrorSuccess)
            {
                bResult = TRUE;
            }
        }
        else
        {
#if !defined(NDEBUG)
            lDumpNonMatchingPacketID(pCommandData, pResponseData);
#endif

            bResult = FALSE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

BOOL UsbPepCtrlSetVppMode(
  _In_ UINT32 nVppMode)
{
    BOOL bResult = FALSE;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pWriteOverlappedData == NULL ||
        l_DeviceData.UsbDeviceData.pReadOverlappedData == NULL)
    {
        ReleaseMutex(l_DeviceData.hDeviceMutex);

        return FALSE;
    }

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    lIncrementPacketID(pCommandData);

    pCommandData->Command = CPepSetVppModeCommand;
    pCommandData->Data.nVppMode = nVppMode;

    if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                           CDefReceiveTimeoutMs))
    {
        if (pResponseData->PacketID == pCommandData->PacketID)
        {
            if (pResponseData->ErrorCode == CPepErrorSuccess)
            {
                bResult = TRUE;
            }
        }
        else
        {
#if !defined(NDEBUG)
            lDumpNonMatchingPacketID(pCommandData, pResponseData);
#endif

            bResult = FALSE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

_Success_(return)
BOOL UsbPepCtrlReadData(
  _In_ UINT32 nAddress,
  _Out_writes_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    BOOL bResult = TRUE;
    UINT32 nIndex = 0;
    UINT32 nTotal, nResponseDataIndex;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pWriteOverlappedData == NULL ||
        l_DeviceData.UsbDeviceData.pReadOverlappedData == NULL)
    {
        ReleaseMutex(l_DeviceData.hDeviceMutex);

        return FALSE;
    }

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    lIncrementPacketID(pCommandData);

    pCommandData->Command = CPepReadDataCommand;

    while (bResult && nIndex < nDataLen)
    {
        nTotal = nDataLen - nIndex;

        if (nTotal > MArrayLen(pResponseData->Data.nData))
        {
            nTotal = MArrayLen(pResponseData->Data.nData);
        }

        pCommandData->Data.ReadData.nAddress = nAddress + nIndex;
        pCommandData->Data.ReadData.nDataLen = (UINT8)nTotal;

        if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                               CDefReceiveTimeoutMs))
        {
            if (pResponseData->PacketID == pCommandData->PacketID)
            {
                if (pResponseData->ErrorCode == CPepErrorSuccess)
                {
                    for (nResponseDataIndex = 0; nResponseDataIndex < pCommandData->Data.ReadData.nDataLen; ++nResponseDataIndex)
                    {
                        *(pbyData + nIndex + nResponseDataIndex) = pResponseData->Data.nData[nResponseDataIndex];
                    }

                    nIndex += pCommandData->Data.ReadData.nDataLen;
                }
                else
                {
                    bResult = FALSE;
                }
            }
            else
            {
#if !defined(NDEBUG)
                lDumpNonMatchingPacketID(pCommandData, pResponseData);
#endif

                bResult = FALSE;
            }
        }
        else
        {
            bResult = FALSE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

_Success_(return)
BOOL UsbPepCtrlReadUserData(
  _Const_ _In_reads_(nReadUserDataLen) const TUtPepCtrlReadUserData* pReadUserData,
  _In_ UINT32 nReadUserDataLen,
  _Out_writes_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    BOOL bResult = TRUE;
    UINT32 nReadUserDataIndex = 0;
    UINT32 nDataIndex = 0;
    UINT32 nIndex, nTotal;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    nDataLen;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pWriteOverlappedData == NULL ||
        l_DeviceData.UsbDeviceData.pReadOverlappedData == NULL)
    {
        ReleaseMutex(l_DeviceData.hDeviceMutex);

        return FALSE;
    }

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    lIncrementPacketID(pCommandData);

    pCommandData->Command = CPepReadUserDataCommand;

    while (bResult && nReadUserDataIndex < nReadUserDataLen)
    {
        nTotal = nReadUserDataLen - nReadUserDataIndex;

        if (nTotal > MArrayLen(pCommandData->Data.ReadUserData.Data))
        {
            nTotal = MArrayLen(pCommandData->Data.ReadUserData.Data);
        }

        pCommandData->Data.ReadUserData.nDataLen = (UINT8)nTotal;

        for (nIndex = 0; nIndex < pCommandData->Data.ReadUserData.nDataLen; ++nIndex)
        {
            pCommandData->Data.ReadUserData.Data[nIndex].nAddress = pReadUserData[nReadUserDataIndex + nIndex].nAddress;
            pCommandData->Data.ReadUserData.Data[nIndex].nEnableOutputEnable = pReadUserData[nReadUserDataIndex + nIndex].OutputEnableMode == eUtPepCtrlEnableOE;
            pCommandData->Data.ReadUserData.Data[nIndex].nPerformRead = (UINT8)pReadUserData[nReadUserDataIndex + nIndex].bPerformRead;
        }

        if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                               CDefReceiveTimeoutMs))
        {
            if (pResponseData->PacketID == pCommandData->PacketID)
            {
                if (pResponseData->ErrorCode == CPepErrorSuccess)
                {
                    for (nIndex = 0; nIndex < pCommandData->Data.ReadUserData.nDataLen; ++nIndex)
                    {
                        if (pCommandData->Data.ReadUserData.Data[nIndex].nPerformRead)
                        {
                            *(pbyData + nDataIndex) = pResponseData->Data.nData[nIndex];

                            ++nDataIndex;
                        }

                        ++nReadUserDataIndex;
                    }
                }
                else
                {
                    bResult = FALSE;
                }
            }
            else
            {
#if !defined(NDEBUG)
                lDumpNonMatchingPacketID(pCommandData, pResponseData);
#endif

                bResult = FALSE;
            }
        }
        else 
        {
            bResult = FALSE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

_Success_(return)
BOOL UsbPepCtrlReadUserDataWithDelay(
  _Const_ _In_reads_(nReadUserDataWithDelayLen) const TUtPepCtrlReadUserDataWithDelay* pReadUserDataWithDelay,
  _In_ UINT32 nReadUserDataWithDelayLen,
  _Out_writes_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    BOOL bResult = TRUE;
    UINT32 nReadUserDataWithDelayIndex = 0;
    UINT32 nDataIndex = 0;
    UINT32 nIndex, nTotal;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    nDataLen;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pWriteOverlappedData == NULL ||
        l_DeviceData.UsbDeviceData.pReadOverlappedData == NULL)
    {
        ReleaseMutex(l_DeviceData.hDeviceMutex);

        return FALSE;
    }

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    lIncrementPacketID(pCommandData);

    pCommandData->Command = CPepReadUserDataWithDelayCommand;

    while (bResult && nReadUserDataWithDelayIndex < nReadUserDataWithDelayLen)
    {
        nTotal = nReadUserDataWithDelayLen - nReadUserDataWithDelayIndex;

        if (nTotal > MArrayLen(pCommandData->Data.ReadUserDataWithDelay.Data))
        {
            nTotal = MArrayLen(pCommandData->Data.ReadUserDataWithDelay.Data);
        }

        pCommandData->Data.ReadUserDataWithDelay.nDataLen = (UINT8)nTotal;

        for (nIndex = 0; nIndex < pCommandData->Data.ReadUserDataWithDelay.nDataLen; ++nIndex)
        {
            pCommandData->Data.ReadUserDataWithDelay.Data[nIndex].nAddress = pReadUserDataWithDelay[nReadUserDataWithDelayIndex + nIndex].nAddress;
            pCommandData->Data.ReadUserDataWithDelay.Data[nIndex].nDelayNanoSeconds = pReadUserDataWithDelay[nReadUserDataWithDelayIndex + nIndex].nDelayNanoSeconds;
            pCommandData->Data.ReadUserDataWithDelay.Data[nIndex].nPerformRead = (UINT8)pReadUserDataWithDelay[nReadUserDataWithDelayIndex + nIndex].bPerformRead;
        }

        if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                               CDefReceiveTimeoutMs))
        {
            if (pResponseData->PacketID == pCommandData->PacketID)
            {
                if (pResponseData->ErrorCode == CPepErrorSuccess)
                {
                    for (nIndex = 0; nIndex < pCommandData->Data.ReadUserDataWithDelay.nDataLen; ++nIndex)
                    {
                        if (pCommandData->Data.ReadUserDataWithDelay.Data[nIndex].nPerformRead)
                        {
                            *(pbyData + nDataIndex) = pResponseData->Data.nData[nIndex];

                            ++nDataIndex;
                        }

                        ++nReadUserDataWithDelayIndex;
                    }
                }
                else
                {
                    bResult = FALSE;
                }
            }
            else
            {
#if !defined(NDEBUG)
                lDumpNonMatchingPacketID(pCommandData, pResponseData);
#endif

                bResult = FALSE;
            }
        }
        else
        {
            bResult = FALSE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

BOOL UsbPepCtrlProgramData(
  _In_ UINT nAddress,
  _Const_ _In_reads_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    BOOL bResult = TRUE;
    UINT32 nIndex = 0;
    UINT32 nTotal;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pWriteOverlappedData == NULL ||
        l_DeviceData.UsbDeviceData.pReadOverlappedData == NULL)
    {
        ReleaseMutex(l_DeviceData.hDeviceMutex);

        return FALSE;
    }

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    lIncrementPacketID(pCommandData);

    pCommandData->Command = CPepProgramDataCommand;
    pCommandData->Data.ProgramData.nAddress = nAddress;

    while (bResult && nIndex < nDataLen)
    {
        nTotal = nDataLen - nIndex;

        if (nTotal > MArrayLen(pCommandData->Data.ProgramData.nData))
        {
            nTotal = MArrayLen(pCommandData->Data.ProgramData.nData);
        }

        pCommandData->Data.ProgramData.nDataLen = (UINT8)nTotal;

        CopyMemory(pCommandData->Data.ProgramData.nData, pbyData + nIndex,
                   pCommandData->Data.ProgramData.nDataLen);

        if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                               CDefReceiveTimeoutMs))
        {
            if (pResponseData->PacketID == pCommandData->PacketID)
            {
                if (pResponseData->ErrorCode == CPepErrorSuccess)
                {
                    pCommandData->Data.ProgramData.nAddress += pCommandData->Data.ProgramData.nDataLen;

                    nIndex += pCommandData->Data.ProgramData.nDataLen;
                }
                else
                {
                    bResult = FALSE;
                }
            }
            else
            {
#if !defined(NDEBUG)
                lDumpNonMatchingPacketID(pCommandData, pResponseData);
#endif

                bResult = FALSE;
            }
        }
        else
        {
            bResult = FALSE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

BOOL UsbPepCtrlProgramUserData(
  _Const_ _In_reads_(nProgramUserDataLen) const TUtPepCtrlProgramUserData* pProgramUserData,
  _In_ UINT32 nProgramUserDataLen,
  _Const_ _In_reads_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    BOOL bResult = TRUE;
    UINT32 nProgramUserDataIndex = 0;
    UINT32 nDataIndex = 0;
    UINT32 nIndex, nTotal;
    TUtPepCommandData* pCommandData;
    TUtPepResponseData* pResponseData;

    nDataLen;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    if (l_DeviceData.UsbDeviceData.pWriteOverlappedData == NULL ||
        l_DeviceData.UsbDeviceData.pReadOverlappedData == NULL)
    {
        ReleaseMutex(l_DeviceData.hDeviceMutex);

        return FALSE;
    }

    pCommandData = &l_DeviceData.UsbDeviceData.pWriteOverlappedData->Data.CommandData;
    pResponseData = &l_DeviceData.UsbDeviceData.pReadOverlappedData->Data.ResponseData;

    ZeroMemory(pCommandData, sizeof(*pCommandData));

    lIncrementPacketID(pCommandData);

    pCommandData->Command = CPepProgramUserDataCommand;

    while (bResult && nProgramUserDataIndex < nProgramUserDataLen)
    {
        nTotal = nProgramUserDataLen - nProgramUserDataIndex;

        if (nTotal > MArrayLen(pCommandData->Data.ProgramUserData.Data))
        {
            nTotal = MArrayLen(pCommandData->Data.ProgramUserData.Data);
        }

        pCommandData->Data.ProgramUserData.nDataLen = (UINT8)nTotal;

        for (nIndex = 0; nIndex < pCommandData->Data.ProgramUserData.nDataLen; ++nIndex)
        {
            pCommandData->Data.ProgramUserData.Data[nIndex].nAddress = pProgramUserData[nProgramUserDataIndex + nIndex].nAddress;
            pCommandData->Data.ProgramUserData.Data[nIndex].nPerformProgram = (UINT8)pProgramUserData[nProgramUserDataIndex + nIndex].bPerformProgram;

            if (pCommandData->Data.ProgramUserData.Data[nIndex].nPerformProgram)
            {
                pCommandData->Data.ProgramUserData.Data[nIndex].nData = *(pbyData + nDataIndex);

                ++nDataIndex;
            }
        }

        if (lSendDeviceCommand(&l_DeviceData.UsbDeviceData, CDefSendTimeoutMs,
                               CDefReceiveTimeoutMs))
        {
            if (pResponseData->PacketID == pCommandData->PacketID)
            {
                if (pResponseData->ErrorCode == CPepErrorSuccess)
                {
                    nProgramUserDataIndex += pCommandData->Data.ProgramUserData.nDataLen;
                }
                else
                {
                    bResult = FALSE;
                }
            }
            else
            {
#if !defined(NDEBUG)
                lDumpNonMatchingPacketID(pCommandData, pResponseData);
#endif

                bResult = FALSE;
            }
        }
        else
        {
            bResult = FALSE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/
