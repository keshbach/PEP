/***************************************************************************/
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <windows.h>

#include <Config/UtPepCtrl.h>

#include <UtilsPep/UtPepLogicDefs.h>

#include <Utils/UtHeap.h>

#include "ParallelPortPepCtrl.h"
#include "UsbPepCtrl.h"

#pragma region "Type Definitions"

typedef BOOL (*TInitializeFunc)(_In_ TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc);
typedef BOOL (*TUninitializeFunc)(VOID);
typedef BOOL (*TGetDeviceNameFunc)(_Out_ LPWSTR pszDeviceName, _Out_ LPINT pnDeviceNameLen);
typedef BOOL (*TSetDeviceNameFunc)(_Const_ _In_ LPCWSTR pszDeviceName);
typedef BOOL (*TSetDelaySettingsFunc)(_In_ UINT32 nChipEnableNanoSeconds, _In_ UINT32 nOutputEnableNanoSeconds);
typedef BOOL (*TIsDevicePresentFunc)(_Out_writes_(sizeof(BOOL)) LPBOOL pbPresent);
typedef BOOL (*TReset)(VOID);
typedef BOOL (*TSetProgrammerModeFunc)(_In_ UINT32 nProgrammerMode);
typedef BOOL (*TSetVccModeFunc)(_In_ UINT32 nVccMode);
typedef BOOL (*TSetPinPulseModeFunc)(_In_ UINT32 nPinPulseMode);
typedef BOOL (*TSetVppModeFunc)(_In_ UINT32 nVppMode);
typedef BOOL (*TReadDataFunc)(_In_ UINT32 nAddress, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);
typedef BOOL (*TReadUserDataFunc)(_Const_ _In_reads_(nReadUserDataLen) const TUtPepCtrlReadUserData* pReadUserData, _In_ UINT32 nReadUserDataLen, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);
typedef BOOL (*TReadUserDataWithDelayFunc)(_Const_ _In_reads_(nReadUserDataWithDelayLen) const TUtPepCtrlReadUserDataWithDelay* pReadUserDataWithDelay, _In_ UINT32 nReadUserDataWithDelayLen, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);
typedef BOOL (*TProgramDataFunc)(_In_ UINT nAddress, _Const_ _In_reads_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);
typedef BOOL (*TProgramUserDataFunc)(_Const_ _In_reads_(nProgramUserDataLen) const TUtPepCtrlProgramUserData* pProgramUserData, _In_ UINT32 nProgramUserDataLen, _Const_ _In_reads_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

#pragma endregion

#pragma region "Structures"

typedef struct tagTDeviceFuncs
{
    TInitializeFunc pInitializeFunc;
    TUninitializeFunc pUninitializeFunc;
    TGetDeviceNameFunc pGetDeviceNameFunc;
    TSetDeviceNameFunc pSetDeviceNameFunc;
    TSetDelaySettingsFunc pSetDelaySettingsFunc;
    TIsDevicePresentFunc pIsDevicePresentFunc;
    TReset pResetFunc;
    TSetProgrammerModeFunc pSetProgrammerModeFunc;
    TSetVccModeFunc pSetVccModeFunc;
    TSetPinPulseModeFunc pSetPinPulseModeFunc;
    TSetVppModeFunc pSetVppModeFunc;
    TReadDataFunc pReadDataFunc;
    TReadUserDataFunc pReadUserDataFunc;
    TReadUserDataWithDelayFunc pReadUserDataWithDelayFunc;
    TProgramDataFunc pProgramDataFunc;
    TProgramUserDataFunc pProgramUserDataFunc;
} TDeviceFuncs;

#pragma endregion

#pragma region "Local Variables"

static BOOL l_bInitialized = FALSE;
static TDeviceFuncs l_DeviceFuncs = {NULL};

#pragma endregion

BOOL UTPEPCTRLAPI UtPepCtrlInitialize(
  _In_ EUtPepCtrlDeviceType DeviceType,
  _In_ TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc)
{
    if (l_bInitialized)
    {
        return TRUE;
    }
    
    if (UtInitHeap() == FALSE)
    {
        return FALSE;
    }

    switch (DeviceType)
    {
        case eUtPepCtrlParallelPortDeviceType:
            l_DeviceFuncs.pInitializeFunc = &ParallelPortPepCtrlInitialize;
            l_DeviceFuncs.pUninitializeFunc = &ParallelPortPepCtrlUninitialize;
            l_DeviceFuncs.pGetDeviceNameFunc = &ParallelPortPepCtrlGetDeviceName;
            l_DeviceFuncs.pSetDeviceNameFunc = &ParallelPortPepCtrlSetDeviceName;
            l_DeviceFuncs.pSetDelaySettingsFunc = &ParallelPortPepCtrlSetDelaySettings;
            l_DeviceFuncs.pIsDevicePresentFunc = &ParallelPortPepCtrlIsDevicePresent;
            l_DeviceFuncs.pResetFunc = &ParallelPortPepCtrlReset;
            l_DeviceFuncs.pSetProgrammerModeFunc = &ParallelPortPepCtrlSetProgrammerMode;
            l_DeviceFuncs.pSetVccModeFunc = &ParallelPortPepCtrlSetVccMode;
            l_DeviceFuncs.pSetPinPulseModeFunc = &ParallelPortPepCtrlSetPinPulseMode;
            l_DeviceFuncs.pSetVppModeFunc = &ParallelPortPepCtrlSetVppMode;
            l_DeviceFuncs.pReadDataFunc = &ParallelPortPepCtrlReadData;
            l_DeviceFuncs.pReadUserDataFunc = &ParallelPortPepCtrlReadUserData;
            l_DeviceFuncs.pReadUserDataWithDelayFunc = &ParallelPortPepCtrlReadUserDataWithDelay;
            l_DeviceFuncs.pProgramDataFunc = &ParallelPortPepCtrlProgramData;
            l_DeviceFuncs.pProgramUserDataFunc = &ParallelPortPepCtrlProgramUserData;
            break;
        case eUtPepCtrlUsbDeviceType:
            l_DeviceFuncs.pInitializeFunc = &UsbPepCtrlInitialize;
            l_DeviceFuncs.pUninitializeFunc = &UsbPepCtrlUninitialize;
            l_DeviceFuncs.pGetDeviceNameFunc = &UsbPepCtrlGetDeviceName;
            l_DeviceFuncs.pSetDeviceNameFunc = &UsbPepCtrlSetDeviceName;
            l_DeviceFuncs.pSetDelaySettingsFunc = &UsbPepCtrlSetDelaySettings;
            l_DeviceFuncs.pIsDevicePresentFunc = &UsbPepCtrlIsDevicePresent;
            l_DeviceFuncs.pResetFunc = &UsbPepCtrlReset;
            l_DeviceFuncs.pSetProgrammerModeFunc = &UsbPepCtrlSetProgrammerMode;
            l_DeviceFuncs.pSetVccModeFunc = &UsbPepCtrlSetVccMode;
            l_DeviceFuncs.pSetPinPulseModeFunc = &UsbPepCtrlSetPinPulseMode;
            l_DeviceFuncs.pSetVppModeFunc = &UsbPepCtrlSetVppMode;
            l_DeviceFuncs.pReadDataFunc = &UsbPepCtrlReadData;
            l_DeviceFuncs.pReadUserDataFunc = &UsbPepCtrlReadUserData;
            l_DeviceFuncs.pReadUserDataWithDelayFunc = &UsbPepCtrlReadUserDataWithDelay;
            l_DeviceFuncs.pProgramDataFunc = &UsbPepCtrlProgramData;
            l_DeviceFuncs.pProgramUserDataFunc = &UsbPepCtrlProgramUserData;
            break;
        default:
            UtUninitHeap();

            return FALSE;
    }

    if (!l_DeviceFuncs.pInitializeFunc(pDeviceChangeFunc))
    {
        UtUninitHeap();

        return FALSE;
    }

    l_bInitialized = TRUE;

    return TRUE;
}

BOOL UTPEPCTRLAPI UtPepCtrlUninitialize(VOID)
{
    BOOL bResult;

    if (!l_bInitialized)
    {
        return FALSE;
    }

    bResult = l_DeviceFuncs.pUninitializeFunc();

    UtUninitHeap();

    l_bInitialized = FALSE;

    return bResult;
}

BOOL UTPEPCTRLAPI UtPepCtrlGetDeviceName(
    _Out_ LPWSTR pszDeviceName,
    _Out_ LPINT pnDeviceNameLen)
{
    if (pszDeviceName)
    {
        *pszDeviceName = 0;
    }

    if (pnDeviceNameLen)
    {
        *pnDeviceNameLen = 0;
    }

    if (!l_bInitialized)
    {
        return FALSE;
    }

    return l_DeviceFuncs.pGetDeviceNameFunc(pszDeviceName, pnDeviceNameLen);
}

BOOL UTPEPCTRLAPI UtPepCtrlSetDeviceName(
  _Const_ _In_ LPCWSTR pszDeviceName)
{
    BOOL bResult = FALSE;

    if (l_bInitialized == TRUE)
    {
        bResult = l_DeviceFuncs.pSetDeviceNameFunc(pszDeviceName);
    }

    return bResult;
}

BOOL UTPEPCTRLAPI UtPepCtrlSetDelaySettings(
  _In_ UINT32 nChipEnableNanoSeconds,
  _In_ UINT32 nOutputEnableNanoSeconds)
{
    if (!l_bInitialized)
    {
        return FALSE;
    }

    return l_DeviceFuncs.pSetDelaySettingsFunc(nChipEnableNanoSeconds, nOutputEnableNanoSeconds);
}

_Success_(return)
BOOL UTPEPCTRLAPI UtPepCtrlIsDevicePresent(
  _Out_writes_(sizeof(BOOL)) LPBOOL pbPresent)
{
    if (!l_bInitialized)
    {
        return FALSE;
    }

    return l_DeviceFuncs.pIsDevicePresentFunc(pbPresent);
}

BOOL UTPEPCTRLAPI UtPepCtrlReset(VOID)
{
    if (!l_bInitialized)
    {
        return FALSE;
    }

    return l_DeviceFuncs.pResetFunc();
}

BOOL UTPEPCTRLAPI UtPepCtrlSetProgrammerMode(
  _In_ EUtPepCtrlProgrammerMode ProgrammerMode)
{
    UINT32 nProgrammerMode;

    if (!l_bInitialized)
    {
        return FALSE;
    }

    switch (ProgrammerMode)
    {
        case eUtPepCtrlProgrammerNoneMode:
            nProgrammerMode = CUtPepLogicProgrammerNoneMode;
            break;
        case eUtPepCtrlProgrammerReadMode:
            nProgrammerMode = CUtPepLogicProgrammerReadMode;
            break;
        case eUtPepCtrlProgrammerWriteMode:
            nProgrammerMode = CUtPepLogicProgrammerWriteMode;
            break;
        default:
            return FALSE;
    }

    return l_DeviceFuncs.pSetProgrammerModeFunc(nProgrammerMode);
}

BOOL UTPEPCTRLAPI UtPepCtrlSetVccMode(
  _In_ EUtPepCtrlVccMode VccMode)
{
    UINT32 nVccMode;

    if (!l_bInitialized)
    {
        return FALSE;
    }

    switch (VccMode)
    {
        case eUtPepCtrl5VDCMode:
            nVccMode = CUtPepLogic5VDCMode;
            break;
        case eUtPepCtrl625VDCMode:
            nVccMode = CUtPepLogic625VDCMode;
            break;
        default:
            return FALSE;
    }

    return l_DeviceFuncs.pSetVccModeFunc(nVccMode);
}

BOOL UTPEPCTRLAPI UtPepCtrlSetPinPulseMode(
  _In_ EUtPepCtrlPinPulseMode PinPulseMode)
{
    UINT32 nPinPulseMode;

    if (!l_bInitialized)
    {
        return FALSE;
    }

    switch (PinPulseMode)
    {
        case eUtPepCtrlPinPulse1Mode:
            nPinPulseMode = CUtPepLogicPinPulse1Mode;
            break;
        case eUtPepCtrlPinPulse2Mode:
            nPinPulseMode = CUtPepLogicPinPulse2Mode;
            break;
        case eUtPepCtrlPinPulse3Mode:
            nPinPulseMode = CUtPepLogicPinPulse3Mode;
            break;
        case eUtPepCtrlPinPulse4Mode:
            nPinPulseMode = CUtPepLogicPinPulse4Mode;
            break;
        default:
            return FALSE;
    }

    return l_DeviceFuncs.pSetPinPulseModeFunc(nPinPulseMode);
}

BOOL UTPEPCTRLAPI UtPepCtrlSetVppMode(
  _In_ EUtPepCtrlVppMode VppMode)
{
    UINT32 nVppMode;

    if (!l_bInitialized)
    {
        return FALSE;
    }

    switch (VppMode)
    {
        case eUtPepCtrl12VDCVppMode:
            nVppMode = CUtPepLogic12VDCVppMode;
            break;
        case eUtPepCtrl21VDCVppMode:
            nVppMode = CUtPepLogic21VDCVppMode;
            break;
        case eUtPepCtrl25VDCVppMode:
            nVppMode = CUtPepLogic25VDCVppMode;
            break;
        default:
            return FALSE;
    }

    return l_DeviceFuncs.pSetVppModeFunc(nVppMode);
}

_Success_(return)
BOOL UTPEPCTRLAPI UtPepCtrlReadData(
  _In_ UINT32 nAddress,
  _Out_writes_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    if (!l_bInitialized)
    {
        return FALSE;
    }

    return l_DeviceFuncs.pReadDataFunc(nAddress, pbyData, nDataLen);
}

_Success_(return)
BOOL UTPEPCTRLAPI UtPepCtrlReadUserData(
  _Const_ _In_reads_(nReadUserDataLen) const TUtPepCtrlReadUserData* pReadUserData,
  _In_ UINT32 nReadUserDataLen,
  _Out_writes_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    UINT32 nTotalReadUserData = 0;
    UINT32 nIndex;

    if (!l_bInitialized)
    {
        return FALSE;
    }

    for (nIndex = 0; nIndex < nReadUserDataLen; ++nIndex)
    {
        if (pReadUserData[nIndex].bPerformRead)
        {
            ++nTotalReadUserData;
        }
    }

    if (nTotalReadUserData != nDataLen)
    {
        return FALSE;
    }

    return l_DeviceFuncs.pReadUserDataFunc(pReadUserData, nReadUserDataLen, pbyData, nDataLen);
}

_Success_(return)
BOOL UTPEPCTRLAPI UtPepCtrlReadUserDataWithDelay(
  _Const_ _In_reads_(nReadUserDataWithDelayLen) const TUtPepCtrlReadUserDataWithDelay* pReadUserDataWithDelay,
  _In_ UINT32 nReadUserDataWithDelayLen,
  _Out_writes_(nDataLen) LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    UINT32 nTotalReadUserDataWithDelay = 0;
    UINT32 nIndex;

    if (!l_bInitialized)
    {
        return FALSE;
    }

    for (nIndex = 0; nIndex < nReadUserDataWithDelayLen; ++nIndex)
    {
        if (pReadUserDataWithDelay[nIndex].bPerformRead)
        {
            ++nTotalReadUserDataWithDelay;
        }
    }

    if (nTotalReadUserDataWithDelay != nDataLen)
    {
        return FALSE;
    }

    return l_DeviceFuncs.pReadUserDataWithDelayFunc(pReadUserDataWithDelay, nReadUserDataWithDelayLen, pbyData, nDataLen);
}

BOOL UTPEPCTRLAPI UtPepCtrlProgramData(
  _In_ UINT nAddress,
  _Const_ _In_reads_(nDataLen) const LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    if (!l_bInitialized)
    {
        return FALSE;
    }

    return l_DeviceFuncs.pProgramDataFunc(nAddress, pbyData, nDataLen);
}

BOOL UTPEPCTRLAPI UtPepCtrlProgramUserData(
  _Const_ _In_reads_(nProgramUserDataLen) const TUtPepCtrlProgramUserData* pProgramUserData,
  _In_ UINT32 nProgramUserDataLen,
  _Const_ _In_reads_(nDataLen) const LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    UINT32 nTotalProgramData = 0;
    UINT32 nIndex;

    if (!l_bInitialized)
    {
        return FALSE;
    }

    for (nIndex = 0; nIndex < nProgramUserDataLen; ++nIndex)
    {
        if (pProgramUserData[nIndex].bPerformProgram)
        {
            ++nTotalProgramData;
        }
    }

    if (nTotalProgramData != nDataLen)
    {
        return FALSE;
    }

    return l_DeviceFuncs.pProgramUserDataFunc(pProgramUserData, nProgramUserDataLen, pbyData, nDataLen);
}

/***************************************************************************/
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/
