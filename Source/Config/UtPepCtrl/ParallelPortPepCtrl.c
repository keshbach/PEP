/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include <strsafe.h>

#include <winioctl.h>

#include <assert.h>

#include <Drivers/PepCtrlDefs.h>
#include <Drivers/PepCtrlIOCTL.h>

#include <Config/UtPepCtrl.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeap.h>

#include <UtilsPep/UtPepLogicDefs.h>

#include "ParallelPortPepCtrl.h"

#pragma region "Structures"

typedef struct tagTDeviceData
{
    HANDLE hPepCtrl;
    HANDLE hDeviceChangeThread;
    DWORD dwDeviceChangeThreadId;
} TDeviceData;

typedef struct tagTDeviceChangeData
{
    HANDLE hPepCtrl;
    HANDLE hEndThreadEvent;
    TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc;
} TDeviceChangedData;

#pragma endregion

#pragma region "Local Variables"

static TDeviceData l_DeviceData = { INVALID_HANDLE_VALUE, NULL, 0 };
static TDeviceChangedData l_DeviceChangeData = { NULL, NULL, NULL };

#if !defined(NDEBUG)
static DWORD l_dwCurrentThreadId = 0;
#endif

#pragma endregion

#pragma region "Local Functions"

static DWORD WINAPI lDeviceChangeThreadFunc(
  _In_ LPVOID pvParameter)
{
    TDeviceChangedData* pDeviceChangeData = (TDeviceChangedData*)pvParameter;
    BOOL bQuit = FALSE;
    OVERLAPPED Overlapped;
    HANDLE hEvents[2];
    UINT32 nDeviceChange;
    DWORD dwBytesReturned, dwResult;

    ZeroMemory(&Overlapped, sizeof(Overlapped));

    Overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (Overlapped.hEvent == NULL)
    {
        return 0;
    }

    hEvents[0] = Overlapped.hEvent;
    hEvents[1] = pDeviceChangeData->hEndThreadEvent;

    while (bQuit == FALSE)
    {
        nDeviceChange = 0;
        dwBytesReturned = 0;

        if (FALSE == DeviceIoControl(pDeviceChangeData->hPepCtrl,
                                     IOCTL_PEPCTRL_DEVICE_NOTIFICATION,
                                     NULL, 0,
                                     &nDeviceChange, sizeof(nDeviceChange),
                                     &dwBytesReturned,
                                     &Overlapped) &&
            GetLastError() == ERROR_IO_PENDING)
        {
            dwResult = WaitForMultipleObjects(MArrayLen(hEvents),
                                             hEvents, FALSE, INFINITE);

            switch (dwResult)
            {
                case WAIT_OBJECT_0:
                    if (pDeviceChangeData->pDeviceChangeFunc)
                    {
                        switch (nDeviceChange)
                        {
                            case CPepCtrlDeviceArrived:
                                pDeviceChangeData->pDeviceChangeFunc(eUtPepCtrlDeviceArrived);
                                break;
                            case CPepCtrlDeviceRemoved:
                                pDeviceChangeData->pDeviceChangeFunc(eUtPepCtrlDeviceRemoved);
                                break;
                            default:
                                break;
                        }
                    }

                    ResetEvent(Overlapped.hEvent);
                    break;
                case WAIT_OBJECT_0 + 1:
                    CancelIo(pDeviceChangeData->hPepCtrl);

                    bQuit = TRUE;
                    break;
            }
        }
        else
        {
            bQuit = TRUE;
        }
    }

    CloseHandle(Overlapped.hEvent);

    return 1;
}

static TPepCtrlPortSettings* lAllocPortSettings(VOID)
{
    int nPortSettingsLen = sizeof(TPepCtrlPortSettings) + 50;
    BOOL bResult;
    DWORD dwBytesReturned;
    TPepCtrlPortSettings* pPortSettings;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    while (nPortSettingsLen < 32768)
    {
        pPortSettings = (TPepCtrlPortSettings*)UtAllocMem(nPortSettingsLen);

        if (pPortSettings == NULL)
        {
            return NULL;
        }

        bResult = DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_GET_PORT_SETTINGS,
                                  NULL, 0,
                                  pPortSettings, nPortSettingsLen,
                                  &dwBytesReturned, NULL);

        if (bResult && dwBytesReturned >= sizeof(TPepCtrlPortSettings))
        {
            return pPortSettings;
        }

        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            UtFreeMem(pPortSettings);

            return NULL;
        }

        nPortSettingsLen += 50;

        UtFreeMem(pPortSettings);
    }

    return NULL;
}

static VOID lFreePortSettings(
  _In_ TPepCtrlPortSettings* pPortSettings)
{
    UtFreeMem(pPortSettings);
}

static BOOL lSetAddress(
  _In_ UINT32 nAddress)
{
    DWORD dwBytesReturned;

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_ADDRESS,
                         &nAddress, sizeof(nAddress), NULL, 0,
                         &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL lSetAddressWithDelay(
  _In_ UINT32 nAddress,
  _In_ UINT32 nDelayNanoSeconds)
{
    DWORD dwBytesReturned;
    TPepCtrlAddressWithDelay AddressWithDelay;

    AddressWithDelay.nAddress = nAddress;
    AddressWithDelay.nDelayNanoseconds = nDelayNanoSeconds;

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_ADDRESS_WITH_DELAY,
                         &AddressWithDelay, sizeof(AddressWithDelay), NULL, 0,
                         &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

_Success_(return)
static BOOL lGetData(
  _Out_ UINT8 * pnData)
{
    DWORD dwBytesReturned;

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_GET_DATA,
                         NULL, 0, pnData, sizeof(*pnData),
                         &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL lSetData(
  _In_ UINT8 nData)
{
    DWORD dwBytesReturned;

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_DATA,
                         &nData, sizeof(nData), NULL, 0,
                         &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

_Success_(return)
static BOOL lTriggerProgram(
  _Out_ LPBOOL pbProgramSuccess)
{
    DWORD dwBytesReturned;
    UINT32 nProgramSuccess;

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_TRIGGER_PROGRAM,
                         NULL, 0, &nProgramSuccess, sizeof(nProgramSuccess),
                         &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    *pbProgramSuccess = nProgramSuccess;

    return TRUE;
}

static BOOL lSetOutputEnable(
  _In_ BOOL bEnable)
{
    DWORD dwBytesReturned;
    UINT32 nEnableOutputEnable;

    nEnableOutputEnable = bEnable;

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_OUTPUT_ENABLE,
                         &nEnableOutputEnable, sizeof(nEnableOutputEnable),
                         NULL, 0, &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL lSetDebugWritePortData(
  _In_ UINT8 nWritePortData)
{
    DWORD dwBytesReturned;

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_DEBUG_WRITE_PORT_DATA,
                         &nWritePortData, sizeof(nWritePortData),
                         NULL, 0, &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

#pragma endregion

BOOL ParallelPortPepCtrlInitialize(
  _In_ TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc)
{
    l_DeviceChangeData.pDeviceChangeFunc = pDeviceChangeFunc;

    l_DeviceData.hPepCtrl = CreateFile(CPepCtrlDeviceName, GENERIC_READ,
                                       0, NULL, OPEN_EXISTING,
                                       FILE_FLAG_OVERLAPPED, NULL);

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    l_DeviceChangeData.hPepCtrl = l_DeviceData.hPepCtrl;
    l_DeviceChangeData.hEndThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (l_DeviceChangeData.hEndThreadEvent == NULL)
    {
        CloseHandle(l_DeviceData.hPepCtrl);

        return FALSE;
    }

    l_DeviceData.hDeviceChangeThread = CreateThread(NULL, 0, &lDeviceChangeThreadFunc,
                                                    &l_DeviceChangeData, 0,
                                                    &l_DeviceData.dwDeviceChangeThreadId);

    if (l_DeviceData.hDeviceChangeThread == NULL)
    {
        CloseHandle(l_DeviceChangeData.hEndThreadEvent);
        CloseHandle(l_DeviceData.hPepCtrl);

        return FALSE;
    }

#if !defined(NDEBUG)
    l_dwCurrentThreadId = GetCurrentThreadId();
#endif

    return TRUE;
}

BOOL ParallelPortPepCtrlUninitialize(VOID)
{
    assert(l_dwCurrentThreadId == GetCurrentThreadId());

    SetEvent(l_DeviceChangeData.hEndThreadEvent);

    WaitForSingleObject(l_DeviceData.hDeviceChangeThread, INFINITE);

    CloseHandle(l_DeviceData.hDeviceChangeThread);

    CloseHandle(l_DeviceChangeData.hEndThreadEvent);

    CloseHandle(l_DeviceData.hPepCtrl);

#if !defined(NDEBUG)
    l_dwCurrentThreadId = 0;
#endif

    return TRUE;
}

_Success_(return)
BOOL ParallelPortPepCtrlGetDeviceName(
  _Out_ LPWSTR pszDeviceName,
  _Out_ LPINT pnDeviceNameLen)
{
    TPepCtrlPortSettings* pPortSettings;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (pnDeviceNameLen == NULL)
    {
        return FALSE;
    }

    pPortSettings = lAllocPortSettings();

    if (pPortSettings == NULL)
    {
        return FALSE;
    }

    if (pszDeviceName == NULL)
    {
        *pnDeviceNameLen = lstrlenW(pPortSettings->cPortDeviceName) + 1;

        lFreePortSettings(pPortSettings);

        return TRUE;
    }

    if (*pnDeviceNameLen < lstrlenW(pPortSettings->cPortDeviceName) + 1)
    {
        lFreePortSettings(pPortSettings);

        return FALSE;
    }

    StringCchCopyW(pszDeviceName, *pnDeviceNameLen, pPortSettings->cPortDeviceName);

    lFreePortSettings(pPortSettings);

    return TRUE;
}

BOOL ParallelPortPepCtrlSetDeviceName(
  _Const_ _In_ LPCWSTR pszDeviceName)
{
    BOOL bResult;
    DWORD dwBytesReturned;
    int nDeviceNameLen, nPortSettingsLen;
    TPepCtrlPortSettings* pPortSettings;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (pszDeviceName)
    {
        nDeviceNameLen = lstrlenW(pszDeviceName);
    }
    else
    {
        pszDeviceName = L"";
        nDeviceNameLen = 0;
    }

    nPortSettingsLen = sizeof(TPepCtrlPortSettings) + (nDeviceNameLen * sizeof(WCHAR));
    pPortSettings = (TPepCtrlPortSettings*)UtAllocMem(nPortSettingsLen);

    if (pPortSettings == NULL)
    {
        return FALSE;
    }

    ZeroMemory(pPortSettings, nPortSettingsLen);

    if (nDeviceNameLen > 0)
    {
        pPortSettings->nPortType = CPepCtrlParallelPortType;
    }
    else
    {
        pPortSettings->nPortType = CPepCtrlNoPortType;
    }

    StringCchCopyW(pPortSettings->cPortDeviceName, nDeviceNameLen + 1, pszDeviceName);

    bResult = DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_PORT_SETTINGS,
                              pPortSettings, nPortSettingsLen,
                              NULL, 0, &dwBytesReturned,
                              NULL);

    UtFreeMem(pPortSettings);

    return bResult;
}

BOOL ParallelPortPepCtrlSetDelaySettings(
  _In_ UINT32 nChipEnableNanoSeconds,
  _In_ UINT32 nOutputEnableNanoSeconds)
{
    DWORD dwBytesReturned;
    TPepCtrlDelaySettings DelaySettings;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    DelaySettings.nChipEnableNanoseconds = nChipEnableNanoSeconds;
    DelaySettings.nOutputEnableNanoseconds = nOutputEnableNanoSeconds;

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_DELAY_SETTINGS,
                         &DelaySettings, sizeof(DelaySettings), NULL, 0,
                         &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

_Success_(return)
BOOL ParallelPortPepCtrlIsDevicePresent(
  _Out_writes_(sizeof(BOOL)) LPBOOL pbPresent)
{
    DWORD dwBytesReturned;
    UINT32 nDeviceStatus;

    *pbPresent = FALSE;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_GET_DEVICE_STATUS,
                         NULL, 0, &nDeviceStatus, sizeof(nDeviceStatus),
                         &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    if (nDeviceStatus == CPepCtrlDevicePresent)
    {
        *pbPresent = TRUE;
    }

    return TRUE;
}

BOOL ParallelPortPepCtrlReset(VOID)
{
    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    return ParallelPortPepCtrlSetProgrammerMode(CUtPepLogicProgrammerNoneMode) &&
           lSetAddress(0);
}

BOOL ParallelPortPepCtrlSetProgrammerMode(
  _In_ UINT32 nProgrammerMode)
{
    DWORD dwBytesReturned;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_PROGRAMMER_MODE,
                         &nProgrammerMode, sizeof(nProgrammerMode),
                         NULL, 0, &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL ParallelPortPepCtrlSetVccMode(
  _In_ UINT32 nVccMode)
{
    DWORD dwBytesReturned;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_VCC_MODE,
                         &nVccMode, sizeof(nVccMode), NULL, 0,
                         &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL ParallelPortPepCtrlSetPinPulseMode(
  _In_ UINT32 nPinPulseMode)
{
    DWORD dwBytesReturned;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_PIN_PULSE_MODE,
                         &nPinPulseMode, sizeof(nPinPulseMode),
                         NULL, 0, &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL ParallelPortPepCtrlSetVppMode(
  _In_ UINT32 nVppMode)
{
    DWORD dwBytesReturned;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_VPP_MODE,
                         &nVppMode, sizeof(nVppMode),
                         NULL, 0, &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

_Success_(return)
BOOL ParallelPortPepCtrlReadData(
  _In_ UINT32 nAddress,
  _Out_writes_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    UINT32 nIndex;
    
    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    for (nIndex = 0; nIndex < nDataLen; ++nIndex)
    {
        if (!lSetOutputEnable(FALSE) ||
            !lSetAddress(nAddress) ||
            !lSetOutputEnable(TRUE) ||
            !lGetData(pbyData))
        {
            return FALSE;
        }

        ++nAddress;
        ++pbyData;
    }

    return TRUE;
}

_Success_(return)
BOOL ParallelPortPepCtrlReadUserData(
  _Const_ _In_reads_(nReadUserDataLen) const TUtPepCtrlReadUserData* pReadUserData,
  _In_ UINT32 nReadUserDataLen,
  _Out_writes_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    UINT32 nReadUserDataIndex, nDataIndex;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    nDataIndex = 0;

    for (nReadUserDataIndex = 0; nReadUserDataIndex < nReadUserDataLen;
         ++nReadUserDataIndex)
    {
        if (!lSetAddress(pReadUserData[nReadUserDataIndex].nAddress))
        {
            return FALSE;
        }

        switch (pReadUserData[nReadUserDataIndex].OutputEnableMode)
        {
            case eUtPepCtrlEnableOE:
                if (!lSetOutputEnable(TRUE))
                {
                    return FALSE;
                }
                break;
            case eUtPepCtrlDisableOE:
                if (!lSetOutputEnable(FALSE))
                {
                    return FALSE;
                }
                break;
        }

        if (pReadUserData[nReadUserDataIndex].bPerformRead &&
            nDataIndex < nDataLen)
        {
            if (!lGetData(pbyData))
            {
                return FALSE;
            }

            ++nDataIndex;
            ++pbyData;
        }
    }

    return TRUE;
}

_Success_(return)
BOOL ParallelPortPepCtrlReadUserDataWithDelay(
  _Const_ _In_reads_(nReadUserDataWithDelayLen) const TUtPepCtrlReadUserDataWithDelay* pReadUserDataWithDelay,
  _In_ UINT32 nReadUserDataWithDelayLen,
  _Out_writes_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    UINT32 nReadUserDataWithDelayIndex, nDataIndex;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    nDataIndex = 0;

    for (nReadUserDataWithDelayIndex = 0; nReadUserDataWithDelayIndex < nReadUserDataWithDelayLen;
         ++nReadUserDataWithDelayIndex)
    {
        if (!lSetAddressWithDelay(pReadUserDataWithDelay[nReadUserDataWithDelayIndex].nAddress,
                                  pReadUserDataWithDelay[nReadUserDataWithDelayIndex].nDelayNanoSeconds))
        {
            return FALSE;
        }

        if (pReadUserDataWithDelay[nReadUserDataWithDelayIndex].bPerformRead &&
            nDataIndex < nDataLen)
        {
            if (!lGetData(pbyData))
            {
                return FALSE;
            }

            ++nDataIndex;
            ++pbyData;
        }
    }

    return TRUE;
}

BOOL ParallelPortPepCtrlProgramData(
  _In_ UINT nAddress,
  _Const_ _In_reads_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    UINT32 nIndex;
    BOOL bProgramSuccess;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    for (nIndex = 0; nIndex < nDataLen; ++nIndex)
    {
        bProgramSuccess = FALSE;

        if (!lSetAddress(nAddress) ||
            !lSetData(*pbyData) ||
            !lTriggerProgram(&bProgramSuccess) ||
            !bProgramSuccess)
        {
            return FALSE;
        }

        ++nAddress;
        ++pbyData;
    }

    return TRUE;
}

BOOL ParallelPortPepCtrlProgramUserData(
  _Const_ _In_reads_(nProgramUserDataLen) const TUtPepCtrlProgramUserData* pProgramUserData,
  _In_ UINT32 nProgramUserDataLen,
  _Const_ _In_reads_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    UINT32 nProgramUserDataIndex, nDataIndex;
    BOOL bProgramSuccess;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    nDataIndex = 0;

    for (nProgramUserDataIndex = 0; nProgramUserDataIndex < nProgramUserDataLen;
        ++nProgramUserDataIndex)
    {
        if (!lSetAddress(pProgramUserData[nProgramUserDataIndex].nAddress))
        {
            return FALSE;
        }

        if (pProgramUserData[nProgramUserDataIndex].bPerformProgram &&
            nDataIndex < nDataLen)
        {
            bProgramSuccess = FALSE;

            if (!lSetData(*pbyData) ||
                !lTriggerProgram(&bProgramSuccess) ||
                !bProgramSuccess)
            {
                return FALSE;
            }

            ++nDataIndex;
            ++pbyData;
        }
    }

    return TRUE;
}

BOOL ParallelPortPepCtrlDebugWritePortData(
  _In_ UINT8 nWritePortData)
{
    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    return lSetDebugWritePortData(nWritePortData);
}

/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/
