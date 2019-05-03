/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <winioctl.h>

#include <assert.h>

#include <Drivers/PepCtrlDefs.h>
#include <Drivers/PepCtrlIOCTL.h>

#include <UtilsPep/UtPepLogicDefs.h>

#include <Config/UtPepCtrl.h>
#include <Config/UtPepCtrlCfg.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeap.h>

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

static BOOL l_bInitialized = FALSE;
static TDeviceData l_DeviceData = {INVALID_HANDLE_VALUE, NULL, 0};
static TDeviceChangedData l_DeviceChangeData = {NULL, NULL, NULL};
static UINT32 l_nEnableOutputEnable = TRUE;
static UINT32 l_nDisableOutputEnable = FALSE;

#if !defined(NDEBUG)
static DWORD l_dwCurrentThreadId = 0;
#endif

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
                default:
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

BOOL UTPEPCTRLAPI UtPepCtrlInitialize(
  _In_ TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc)
{
    if (l_bInitialized == FALSE)
    {
        if (UtInitHeap() == FALSE)
        {
            return FALSE;
        }

        l_DeviceChangeData.pDeviceChangeFunc = pDeviceChangeFunc;

        l_DeviceData.hPepCtrl = CreateFile(CPepCtrlDeviceName, GENERIC_READ,
                                           0, NULL, OPEN_EXISTING,
                                           FILE_FLAG_OVERLAPPED, NULL);

        if (l_DeviceData.hPepCtrl != INVALID_HANDLE_VALUE)
        {
            l_DeviceChangeData.hPepCtrl = l_DeviceData.hPepCtrl;
            l_DeviceChangeData.hEndThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

            if (l_DeviceChangeData.hEndThreadEvent == NULL)
            {
                CloseHandle(l_DeviceData.hPepCtrl);

                UtUninitHeap();

                return FALSE;
            }

            l_DeviceData.hDeviceChangeThread = CreateThread(NULL, 0, &lDeviceChangeThreadFunc,
                                                            &l_DeviceChangeData, 0,
                                                            &l_DeviceData.dwDeviceChangeThreadId);

            if (l_DeviceData.hDeviceChangeThread == NULL)
            {
                CloseHandle(l_DeviceChangeData.hEndThreadEvent);
                CloseHandle(l_DeviceData.hPepCtrl);

                UtUninitHeap();

                return FALSE;
            }

            l_bInitialized = TRUE;

#if !defined(NDEBUG)
            l_dwCurrentThreadId = GetCurrentThreadId();
#endif

            return TRUE;
        }
    }

    UtUninitHeap();

    return FALSE;
}

BOOL UTPEPCTRLAPI UtPepCtrlUninitialize(VOID)
{
    BOOL bResult = FALSE;

    if (l_bInitialized == TRUE)
    {
        assert(l_dwCurrentThreadId == GetCurrentThreadId());

        SetEvent(l_DeviceChangeData.hEndThreadEvent);

        WaitForSingleObject(l_DeviceData.hDeviceChangeThread, INFINITE);

        CloseHandle(l_DeviceData.hDeviceChangeThread);

        CloseHandle(l_DeviceChangeData.hEndThreadEvent);

        CloseHandle(l_DeviceData.hPepCtrl);

        bResult = UtUninitHeap();

        l_bInitialized = FALSE;

#if !defined(NDEBUG)
        l_dwCurrentThreadId = 0;
#endif
    }

    return bResult;
}

BOOL UTPEPCTRLAPI UtPepCtrlIsDevicePresent(
  _Out_ LPBOOL pbPresent)
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
                         &dwBytesReturned,  NULL))
    {
        return FALSE;
    }

    if (nDeviceStatus == CPepCtrlDevicePresent)
    {
        *pbPresent = TRUE;
    }

    return TRUE;
}

BOOL UTPEPCTRLAPI UtPepCtrlReset(VOID)
{
    UINT32 nAddress = 0;
    DWORD dwBytesReturned;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (!UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode))
    {
        return FALSE;
    }

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_ADDRESS,
                         &nAddress, sizeof(nAddress), NULL, 0,
                         &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL UTPEPCTRLAPI UtPepCtrlSetProgrammerMode(
  _In_ EUtPepCtrlProgrammerMode ProgrammerMode)
{
    DWORD dwBytesReturned;
    UINT32 nProgrammerMode;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
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

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_PROGRAMMER_MODE,
                         &nProgrammerMode, sizeof(nProgrammerMode),
                         NULL, 0, &dwBytesReturned,
                         NULL))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL UTPEPCTRLAPI UtPepCtrlSetVccMode(
  _In_ EUtPepCtrlVccMode VccMode)
{
    DWORD dwBytesReturned;
    UINT32 nVccMode;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
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

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_VCC_MODE,
                         &nVccMode, sizeof(nVccMode), NULL, 0,
                         &dwBytesReturned, NULL))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL UTPEPCTRLAPI UtPepCtrlSetPinPulseMode(
  _In_ EUtPepCtrlPinPulseMode PinPulseMode)
{
    DWORD dwBytesReturned;
    UINT32 nPinPulseMode;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
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

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_PIN_PULSE_MODE,
                         &nPinPulseMode, sizeof(nPinPulseMode),
                         NULL, 0, &dwBytesReturned,
                         NULL))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL UTPEPCTRLAPI UtPepCtrlSetVppMode(
  _In_ EUtPepCtrlVppMode VppMode)
{
    DWORD dwBytesReturned;
    UINT32 nVppMode;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
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

    if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_VPP_MODE,
                         &nVppMode, sizeof(nVppMode),
                         NULL, 0, &dwBytesReturned,
                         NULL))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL UTPEPCTRLAPI UtPepCtrlReadData(
  _In_ UINT32 nAddress,
  _Out_ LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    UINT32 nIndex;
    DWORD dwBytesReturned;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    for (nIndex = 0; nIndex < nDataLen; ++nIndex)
    {
        if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_OUTPUT_ENABLE,
                             &l_nDisableOutputEnable, sizeof(l_nDisableOutputEnable),
                             NULL, 0, &dwBytesReturned, NULL) ||
            !DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_ADDRESS,
                             &nAddress, sizeof(nAddress), NULL, 0,
                             &dwBytesReturned, NULL) ||
            !DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_OUTPUT_ENABLE,
                             &l_nEnableOutputEnable, sizeof(l_nEnableOutputEnable),
                             NULL, 0, &dwBytesReturned, NULL) ||
            !DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_GET_DATA,
                             NULL, 0, pbyData, sizeof(*pbyData),
                             &dwBytesReturned, NULL))
        {
            return FALSE;
        }

        ++nAddress;
        ++pbyData;
    }

    return TRUE;
}

BOOL UTPEPCTRLAPI UtPepCtrlReadUserData(
  _In_ const TUtPepCtrlReadUserData* pReadUserData,
  _In_ UINT32 nReadUserDataLen,
  _Out_ LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    DWORD dwBytesReturned;
    UINT32 nReadUserDataIndex, nDataIndex;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    nDataIndex = 0;

    for (nReadUserDataIndex = 0; nReadUserDataIndex < nReadUserDataLen;
         ++nReadUserDataIndex)
    {
        if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_ADDRESS,
                             (LPVOID)&pReadUserData[nReadUserDataIndex].nAddress,
                             sizeof(pReadUserData[nReadUserDataIndex].nAddress), NULL, 0,
                             &dwBytesReturned, NULL))
        {
            return FALSE;
        }

        switch (pReadUserData[nReadUserDataIndex].OutputEnableMode)
        {
            case eUtPepCtrlIgnoreOE:
                break;
            case eUtPepCtrlEnableOE:
                if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_OUTPUT_ENABLE,
                                     &l_nEnableOutputEnable, sizeof(l_nEnableOutputEnable),
                                     NULL, 0, &dwBytesReturned, NULL))
                {
                    return FALSE;
                }
                break;
            case eUtPepCtrlDisableOE:
                if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_OUTPUT_ENABLE,
                                     &l_nDisableOutputEnable, sizeof(l_nDisableOutputEnable),
                                     NULL, 0, &dwBytesReturned, NULL))
                {
                    return FALSE;
                }
                break;
        }

        if (pReadUserData[nReadUserDataIndex].bPerformRead &&
            nDataIndex < nDataLen)
        {
            if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_GET_DATA,
                                 NULL, 0, pbyData, sizeof(*pbyData),
                                 &dwBytesReturned, NULL))
            {
                return FALSE;
            }

            ++nDataIndex;
            ++pbyData;
        }
    }

    return TRUE;
}

BOOL UTPEPCTRLAPI UtPepCtrlProgramData(
  _In_ UINT nAddress,
  _Out_ LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    UINT32 nIndex;
    DWORD dwBytesReturned;
    UINT32 nProgramSuccess;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    for (nIndex = 0; nIndex < nDataLen; ++nIndex)
    {
        nProgramSuccess = 0;

        if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_ADDRESS,
                             &nAddress, sizeof(nAddress), NULL, 0,
                             &dwBytesReturned, NULL) ||
            !DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_DATA,
                             pbyData, sizeof(*pbyData), NULL, 0,
                             &dwBytesReturned, NULL) ||
            !DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_TRIGGER_PROGRAM,
                             NULL, 0, &nProgramSuccess, sizeof(nProgramSuccess),
                             &dwBytesReturned, NULL) ||
            !nProgramSuccess)
        {
            return FALSE;
        }

        ++nAddress;
        ++pbyData;
    }

    return TRUE;
}

BOOL UTPEPCTRLAPI UtPepCtrlProgramUserData(
  _In_ const TUtPepCtrlProgramUserData* pProgramUserData,
  _In_ UINT32 nProgramUserDataLen,
  _Out_ LPBYTE pbyData,
  _In_ UINT32 nDataLen)
{
    DWORD dwBytesReturned;
    UINT32 nProgramUserDataIndex, nDataIndex;
    UINT32 nProgramSuccess;

    if (l_DeviceData.hPepCtrl == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    nDataIndex = 0;

    for (nProgramUserDataIndex = 0; nProgramUserDataIndex < nProgramUserDataLen;
         ++nProgramUserDataIndex)
    {
        if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_ADDRESS,
                             (LPVOID)&pProgramUserData[nProgramUserDataIndex].nAddress,
                             sizeof(pProgramUserData[nProgramUserDataIndex].nAddress), NULL, 0,
                             &dwBytesReturned, NULL))
        {
            return FALSE;
        }

        if (pProgramUserData[nProgramUserDataIndex].bPerformProgram &&
            nDataIndex < nDataLen)
        {
            nProgramSuccess = 0;

            if (!DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_SET_DATA,
                                 pbyData, sizeof(*pbyData), NULL, 0,
                                 &dwBytesReturned, NULL) ||
                !DeviceIoControl(l_DeviceData.hPepCtrl, IOCTL_PEPCTRL_TRIGGER_PROGRAM,
                                 NULL, 0, &nProgramSuccess, sizeof(nProgramSuccess),
                                 &dwBytesReturned, NULL) ||
                !nProgramSuccess)
            {
                return FALSE;
            }

            ++nDataIndex;
            ++pbyData;
        }
    }

    return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
