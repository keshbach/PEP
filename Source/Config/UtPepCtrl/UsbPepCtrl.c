/***************************************************************************/
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
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

#include "UtPepCtrlUtil.h"

#pragma region "Constants"

#define CUiDeviceNotificationCtrlClass L"DeviceNotificationCtrl"

#define CHidLibrary L"hid.dll"
#define CSetupApiLibrary L"setupapi.dll"

#define CDefSendTimeoutMs 1000
#define CDefReceiveTimeoutMs 1000

#pragma endregion

#pragma region "Type Definitions"

typedef void (__stdcall *TGetHidGuidFunc)( _Out_ LPGUID pHidGuid);
typedef BOOLEAN (__stdcall *TGetPreparsedDataFunc)(_In_ HANDLE HidDeviceObject, _Out_ PHIDP_PREPARSED_DATA* PreparsedData);
typedef NTSTATUS (__stdcall *TGetCapsFunc)(_In_ PHIDP_PREPARSED_DATA PreparsedData, _Out_ PHIDP_CAPS Capabilities);
typedef BOOLEAN (__stdcall *TFreePreparsedDataFunc)(_In_ PHIDP_PREPARSED_DATA PreparsedData);
typedef BOOLEAN (__stdcall *TGetAttributesFunc)(_In_ HANDLE HidDeviceObject, _Out_ PHIDD_ATTRIBUTES Attributes);

typedef HDEVINFO (WINAPI *TSetupDiGetClassDevsFunc)(_In_opt_ CONST GUID* ClassGuid, _In_opt_ PCWSTR Enumerator, _In_opt_ HWND hwndParent, _In_ DWORD Flags);
typedef BOOL (WINAPI *TSetupDiEnumDeviceInterfacesFunc)(_In_ HDEVINFO DeviceInfoSet, _In_opt_ PSP_DEVINFO_DATA DeviceInfoData, _In_ CONST GUID* InterfaceClassGuid, _In_ DWORD MemberIndex, _Out_ PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData);
typedef BOOL (WINAPI *TSetupDiGetDeviceInterfaceDetailFunc)(_In_ HDEVINFO DeviceInfoSet, _In_ PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData, _Out_writes_bytes_to_opt_(DeviceInterfaceDetailDataSize, *RequiredSize) PSP_DEVICE_INTERFACE_DETAIL_DATA_W DeviceInterfaceDetailData, _In_ DWORD DeviceInterfaceDetailDataSize, _Out_opt_ _Out_range_(>= , sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W)) PDWORD RequiredSize, _Out_opt_ PSP_DEVINFO_DATA DeviceInfoData);
typedef BOOL (WINAPI *TSetupDiDestroyDeviceInfoListFunc)(_In_ HDEVINFO DeviceInfoSet);

#pragma endregion

#pragma region "Structures"

typedef struct tagTUsbDeviceData {
    HANDLE* phDeviceMutex;
    HANDLE* phDeviceChangeListMutex;
    EUtPepCtrlDeviceChange** ppPepCtrlDeviceChange;
    LPINT pnPepCtrlDeviceChangeLen;

    HANDLE hDevice;
    INT nInputReportLen;
    GUID DeviceInterfaceGuid;
    LPWSTR pszDeviceName;
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

static TDeviceData l_DeviceData = {0};

static TDeviceChangeData l_DeviceChangeData = {0};

#pragma endregion

#pragma region "Local Functions"

static VOID lGetHidGuid(
  _Out_writes_(sizeof(GUID)) LPGUID pGuid)
{
    HMODULE hModule = LoadLibraryW(CHidLibrary);
    TGetHidGuidFunc pGetHidGuidFunc;

    ZeroMemory(pGuid, sizeof(*pGuid));

    if (hModule == NULL)
    {
        return;
    }

    pGetHidGuidFunc = (TGetHidGuidFunc)GetProcAddress(hModule, "HidD_GetHidGuid");

    if (pGetHidGuidFunc)
    {
        pGetHidGuidFunc(pGuid);
    }

    FreeLibrary(hModule);
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

static void lPushPepCtrlDeviceChange(
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

static HANDLE lOpenDevice(
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

static VOID lCloseDevice(
  _In_ TUsbDeviceData* pUsbDeviceData)
{
    CloseHandle(pUsbDeviceData->hDevice);

    pUsbDeviceData->hDevice = NULL;
}

_Success_(return)
static BOOL lSendDeviceCommand(
  _In_ HANDLE hDevice,
  _In_ INT nTimeoutMs,
  _Const_ _In_ const TUtPepCommandData* pCommandData)
{
    HANDLE hEvent;
    LPOVERLAPPED pOverlapped;
    DWORD dwBytesWritten, dwNumberOfBytesTransferred, dwError;
    BYTE byData[sizeof(TUtPepCommandData) + 1];

    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (hEvent == NULL)
    {
        return FALSE;
    }

    pOverlapped = (LPOVERLAPPED)UtAllocMem(sizeof(OVERLAPPED));

    if (pOverlapped == NULL)
    {
        CloseHandle(hEvent);

        return FALSE;
    }

    byData[0] = 0;

    CopyMemory(byData + 1, pCommandData, sizeof(TUtPepCommandData));

    ZeroMemory(pOverlapped, sizeof(OVERLAPPED));

    pOverlapped->hEvent = hEvent;

    if (FALSE == WriteFile(hDevice, byData, sizeof(byData), &dwBytesWritten, pOverlapped))
    {
        dwError = GetLastError();

        if (dwError == ERROR_IO_PENDING)
        {
            switch (WaitForSingleObject(hEvent, nTimeoutMs))
            {
                case WAIT_OBJECT_0:
                    // Success
                    break;
                case WAIT_TIMEOUT:
                    if (CancelIo(hDevice))
                    {
                        if (GetOverlappedResult(hDevice, pOverlapped, &dwNumberOfBytesTransferred, TRUE))
                        {
                            UtFreeMem(pOverlapped);

                            CloseHandle(hEvent);
                        }
                        else
                        {
                            dwError = GetLastError();

                            if (dwError == ERROR_IO_INCOMPLETE)
                            {
                                UtFreeMem(pOverlapped);

                                CloseHandle(hEvent);
                            }
                        }
                    }

                    return FALSE;
                case WAIT_ABANDONED:
                    break;
                case WAIT_FAILED:
                    dwError = GetLastError();
                    break;
            }
        }
        else
        {
            /*if (dwError == ERROR_INVALID_HANDLE)
            {
                OutputDebugString(L"invalid handle");
            }*/

            UtFreeMem(pOverlapped);

            CloseHandle(hEvent);

            return FALSE;
        }
    }

    UtFreeMem(pOverlapped);

    CloseHandle(hEvent);

    return TRUE;
}

_Success_(return)
static BOOL lReceiveDeviceCommand(
  _In_ HANDLE hDevice,
  _In_ INT nTimeoutMs,
  _Out_ TUtPepResponseData * pResponseData)
{
    HANDLE hEvent;
    LPOVERLAPPED pOverlapped;
    DWORD dwNumberOfBytesTransferred, dwBytesRead, dwError;
    BYTE byData[sizeof(TUtPepCommandData) + 1];

    hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (hEvent == NULL)
    {
        return FALSE;
    }

    pOverlapped = (LPOVERLAPPED)UtAllocMem(sizeof(OVERLAPPED));

    if (pOverlapped == NULL)
    {
        CloseHandle(hEvent);

        return FALSE;
    }

    ZeroMemory(pOverlapped, sizeof(OVERLAPPED));

    pOverlapped->hEvent = hEvent;

    byData[0] = 0;

    if (FALSE == ReadFile(hDevice, byData, sizeof(byData), &dwBytesRead, pOverlapped))
    {
        dwError = GetLastError();

        if (dwError == ERROR_IO_PENDING)
        {
            switch (WaitForSingleObject(hEvent, nTimeoutMs))
            {
                case WAIT_OBJECT_0:
                    // Success
                    break;
                case WAIT_TIMEOUT:
                    if (CancelIo(hDevice))
                    {
                        if (GetOverlappedResult(hDevice, pOverlapped, &dwNumberOfBytesTransferred, TRUE))
                        {
                            UtFreeMem(pOverlapped);

                            CloseHandle(hEvent);
                        }
                        else
                        {
                            dwError = GetLastError();

                            if (dwError == ERROR_IO_INCOMPLETE)
                            {
                                UtFreeMem(pOverlapped);

                                CloseHandle(hEvent);
                            }
                        }
                    }

                    return FALSE;
                case WAIT_ABANDONED:
                    break;
                case WAIT_FAILED:
                    dwError = GetLastError();
                    break;
            }
        }
        else
        {
            /*if (dwError == ERROR_INVALID_HANDLE)
            {
                OutputDebugString(L"invalid handle");
            }*/

            UtFreeMem(pOverlapped);

            CloseHandle(hEvent);

            return FALSE;
        }
    }

    CopyMemory(pResponseData, byData + 1, sizeof(TUtPepResponseData));

    UtFreeMem(pOverlapped);

    CloseHandle(hEvent);

    return TRUE;
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

static BOOL lRefreshDeviceInterfaces(
  _In_ HWND hWnd)
{
    HMODULE hModule = LoadLibraryW(CSetupApiLibrary);
    TSetupDiGetClassDevsFunc pSetupDiGetClassDevsFunc;
    TSetupDiEnumDeviceInterfacesFunc pSetupDiEnumDeviceInterfacesFunc;
    TSetupDiGetDeviceInterfaceDetailFunc pSetupDiGetDeviceInterfaceDetailFunc;
    TSetupDiDestroyDeviceInfoListFunc pSetupDiDestroyDeviceInfoListFunc;
    GUID InterfaceGuid;
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

    lGetHidGuid(&InterfaceGuid);

    hDevInfo = pSetupDiGetClassDevsFunc(&InterfaceGuid, NULL, NULL,
                                        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        FreeLibrary(hModule);

        return FALSE;
    }

    dwDevCount = 0;

    DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);

    while (pSetupDiEnumDeviceInterfacesFunc(hDevInfo, 0, &InterfaceGuid,
                                            dwDevCount, &DeviceInterfaceData))
    {
        dwBufferLen = 0;

        pSetupDiGetDeviceInterfaceDetailFunc(hDevInfo, &DeviceInterfaceData, 0, 0,
                                             &dwBufferLen, 0);

        pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)UtAllocMem(dwBufferLen);

        if (pDeviceInterfaceDetailData == NULL)
        {
            pSetupDiDestroyDeviceInfoListFunc(hDevInfo);

            FreeLibrary(hModule);

            return FALSE;
        }

        pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        if (!pSetupDiGetDeviceInterfaceDetailFunc(hDevInfo, &DeviceInterfaceData,
                                                  pDeviceInterfaceDetailData,
                                                  dwBufferLen, &dwRequiredLen, NULL))
        {
            UtFreeMem(pDeviceInterfaceDetailData);

            pSetupDiDestroyDeviceInfoListFunc(hDevInfo);

            FreeLibrary(hModule);

            return FALSE;
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
            pDevBroadcastDeviceInterface->dbcc_classguid = InterfaceGuid;

            StringCchCopyW(pDevBroadcastDeviceInterface->dbcc_name, nDevicePathLen,
                           pDeviceInterfaceDetailData->DevicePath);

            SendMessage(hWnd, WM_DEVICECHANGE, DBT_DEVICEARRIVAL,
                        (LPARAM)pDevBroadcastDeviceInterface);

            UtFreeMem(pDeviceInterfaceDetailData);
        }

        ++dwDevCount;
    }

    pSetupDiDestroyDeviceInfoListFunc(hDevInfo);

    FreeLibrary(hModule);

    return TRUE;
}

static HDEVNOTIFY lRegisterDeviceInterfaceNotification(
  _In_ HWND hWnd)
{
    DEV_BROADCAST_DEVICEINTERFACE DevBroadcastDeviceInterface;

    ZeroMemory(&DevBroadcastDeviceInterface, sizeof(DevBroadcastDeviceInterface));

    DevBroadcastDeviceInterface.dbcc_size = sizeof(DevBroadcastDeviceInterface);
    DevBroadcastDeviceInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    lGetHidGuid(&DevBroadcastDeviceInterface.dbcc_classguid);

    return RegisterDeviceNotification(hWnd, &DevBroadcastDeviceInterface,
                                      DEVICE_NOTIFY_WINDOW_HANDLE);
}

static VOID lDeviceArrivalBroadcastDeviceInterface(
  _In_ HWND hWnd,
  _In_ PDEV_BROADCAST_DEVICEINTERFACE pDevBroadcastDeviceInterface)
{
    TUsbDeviceData* pUsbDeviceData = (TUsbDeviceData*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    HANDLE hDevice;
    INT nDeviceNameLen;

    WaitForSingleObject(*(pUsbDeviceData->phDeviceMutex), INFINITE);

    if (pUsbDeviceData->hDevice)
    {
        ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

        return;
    }

    hDevice = lOpenDevice(pDevBroadcastDeviceInterface->dbcc_name);

    if (hDevice == NULL)
    {
        ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

        return;
    }

    if (!lGetDeviceInputReportLength(hDevice, &pUsbDeviceData->nInputReportLen))
    {
        ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

        CloseHandle(hDevice);

        return;
    }

    pUsbDeviceData->DeviceInterfaceGuid = pDevBroadcastDeviceInterface->dbcc_classguid;

    nDeviceNameLen = lstrlenW(pDevBroadcastDeviceInterface->dbcc_name) + 1;

    pUsbDeviceData->pszDeviceName = (LPTSTR)UtAllocMem(nDeviceNameLen * sizeof(WCHAR));

    if (pUsbDeviceData->pszDeviceName == NULL)
    {
        ReleaseMutex(*(pUsbDeviceData->phDeviceMutex));

        CloseHandle(hDevice);

        return;
    }

    StringCchCopyW(pUsbDeviceData->pszDeviceName, nDeviceNameLen, pDevBroadcastDeviceInterface->dbcc_name);

    pUsbDeviceData->hDevice = hDevice;

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
#if !defined(NDEBUG)
        ZeroMemory(&pUsbDeviceData->DeviceInterfaceGuid, sizeof(pUsbDeviceData->DeviceInterfaceGuid));

        pUsbDeviceData->nInputReportLen = 0;
#endif

        lCloseDevice(pUsbDeviceData);

        UtFreeMem(pUsbDeviceData->pszDeviceName);

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
    HDEVNOTIFY hDevNotify;

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

    hDevNotify = lRegisterDeviceInterfaceNotification(hWnd);

    if (hDevNotify == NULL)
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

    UnregisterDeviceNotification(hDevNotify);

    DestroyWindow(hWnd);

    lDeviceNotifyCtrlUnregister();

    return 0;
}

#pragma endregion

BOOL UsbPepCtrlInitialize(
  _In_ TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc)
{
    DWORD dwThreadId;

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
    TUtPepCommandData CommandData;
    TUtPepResponseData ResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    CommandData.Command = CPepSetDelaysCommand;
    CommandData.Data.Delays.nChipEnableNanoSeconds = nChipEnableNanoSeconds;
    CommandData.Data.Delays.nOutputEnableNanoSeconds = nOutputEnableNanoSeconds;

    if (lSendDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefSendTimeoutMs, &CommandData) &&
        lReceiveDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefReceiveTimeoutMs, &ResponseData))
    {
        if (ResponseData.ErrorCode == CPepErrorSuccess)
        {
            bResult = TRUE;
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
    TUtPepCommandData CommandData;
    TUtPepResponseData ResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    CommandData.Command = CPepResetCommand;

    if (lSendDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefSendTimeoutMs, &CommandData) &&
        lReceiveDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefReceiveTimeoutMs, &ResponseData))
    {
        if (ResponseData.ErrorCode == CPepErrorSuccess)
        {
            bResult = TRUE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

BOOL UsbPepCtrlSetProgrammerMode(
  _In_ UINT32 nProgrammerMode)
{
    BOOL bResult = FALSE;
    TUtPepCommandData CommandData;
    TUtPepResponseData ResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    CommandData.Command = CPepSetProgrammerModeCommand;
    CommandData.Data.nProgrammerMode = nProgrammerMode;

    if (lSendDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefSendTimeoutMs, &CommandData) &&
        lReceiveDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefReceiveTimeoutMs, &ResponseData))
    {
        if (ResponseData.ErrorCode == CPepErrorSuccess)
        {
            bResult = TRUE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

BOOL UsbPepCtrlSetVccMode(
  _In_ UINT32 nVccMode)
{
    BOOL bResult = FALSE;
    TUtPepCommandData CommandData;
    TUtPepResponseData ResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    CommandData.Command = CPepSetVccModeCommand;
    CommandData.Data.nVccMode = nVccMode;

    if (lSendDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefSendTimeoutMs, &CommandData) &&
        lReceiveDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefReceiveTimeoutMs, &ResponseData))
    {
        if (ResponseData.ErrorCode == CPepErrorSuccess)
        {
            bResult = TRUE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

BOOL UsbPepCtrlSetPinPulseMode(
  _In_ UINT32 nPinPulseMode)
{
    BOOL bResult = FALSE;
    TUtPepCommandData CommandData;
    TUtPepResponseData ResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    CommandData.Command = CPepSetPinPulseModeCommand;
    CommandData.Data.nPinPulseMode = nPinPulseMode;

    if (lSendDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefSendTimeoutMs, &CommandData) &&
        lReceiveDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefReceiveTimeoutMs, &ResponseData))
    {
        if (ResponseData.ErrorCode == CPepErrorSuccess)
        {
            bResult = TRUE;
        }
    }

    ReleaseMutex(l_DeviceData.hDeviceMutex);

    return bResult;
}

BOOL UsbPepCtrlSetVppMode(
  _In_ UINT32 nVppMode)
{
    BOOL bResult = FALSE;
    TUtPepCommandData CommandData;
    TUtPepResponseData ResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    CommandData.Command = CPepSetVppModeCommand;
    CommandData.Data.nVppMode = nVppMode;

    if (lSendDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefSendTimeoutMs, &CommandData) &&
        lReceiveDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefReceiveTimeoutMs, &ResponseData))
    {
        if (ResponseData.ErrorCode == CPepErrorSuccess)
        {
            bResult = TRUE;
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
    TUtPepCommandData CommandData;
    TUtPepResponseData ResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    CommandData.Command = CPepReadDataCommand;

    while (bResult && nIndex < nDataLen)
    {
        nTotal = nDataLen - nIndex;

        if (nTotal > MArrayLen(ResponseData.Data.nData))
        {
            nTotal = MArrayLen(ResponseData.Data.nData);
        }

        CommandData.Data.ReadData.nAddress = nAddress + nIndex;
        CommandData.Data.ReadData.nDataLen = (UINT8)nTotal;

        if (lSendDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefSendTimeoutMs, &CommandData) &&
            lReceiveDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefReceiveTimeoutMs, &ResponseData))
        {
            if (ResponseData.ErrorCode == CPepErrorSuccess)
            {
                for (nResponseDataIndex = 0; nResponseDataIndex < CommandData.Data.ReadData.nDataLen; ++nResponseDataIndex)
                {
                    *(pbyData + nIndex + nResponseDataIndex) = ResponseData.Data.nData[nResponseDataIndex];
                }

                nIndex += CommandData.Data.ReadData.nDataLen;
            }
            else
            {
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
    TUtPepCommandData CommandData;
    TUtPepResponseData ResponseData;

    nDataLen;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    CommandData.Command = CPepReadUserDataCommand;

    while (bResult && nReadUserDataIndex < nReadUserDataLen)
    {
        nTotal = nReadUserDataLen - nReadUserDataIndex;

        if (nTotal > MArrayLen(CommandData.Data.ReadUserData))
        {
            nTotal = MArrayLen(CommandData.Data.ReadUserData);
        }

        CommandData.Data.nReadUserDataLen = (UINT8)nTotal;

        for (nIndex = 0; nIndex < CommandData.Data.nReadUserDataLen; ++nIndex)
        {
            CommandData.Data.ReadUserData[nIndex].nAddress = pReadUserData[nReadUserDataIndex + nIndex].nAddress;
            CommandData.Data.ReadUserData[nIndex].nEnableOutputEnable = pReadUserData[nReadUserDataIndex + nIndex].OutputEnableMode == eUtPepCtrlEnableOE;
            CommandData.Data.ReadUserData[nIndex].nPerformRead = (UINT8)pReadUserData[nReadUserDataIndex + nIndex].bPerformRead;
        }

        if (lSendDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefSendTimeoutMs, &CommandData) &&
            lReceiveDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefReceiveTimeoutMs, &ResponseData))
        {
            if (ResponseData.ErrorCode == CPepErrorSuccess)
            {
                for (nIndex = 0; nIndex < CommandData.Data.nReadUserDataLen; ++nIndex)
                {
                    if (CommandData.Data.ReadUserData[nIndex].nPerformRead)
                    {
                        *(pbyData + nDataIndex) = ResponseData.Data.nData[nIndex];

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
    TUtPepCommandData CommandData;
    TUtPepResponseData ResponseData;

    nDataLen;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    CommandData.Command = CPepReadUserDataWithDelayCommand;

    while (bResult && nReadUserDataWithDelayIndex < nReadUserDataWithDelayLen)
    {
        nTotal = nReadUserDataWithDelayLen - nReadUserDataWithDelayIndex;

        if (nTotal > MArrayLen(CommandData.Data.ReadUserDataWithDelay))
        {
            nTotal = MArrayLen(CommandData.Data.ReadUserDataWithDelay);
        }

        CommandData.Data.nReadUserDataWithDelayLen = (UINT8)nTotal;

        for (nIndex = 0; nIndex < CommandData.Data.nReadUserDataWithDelayLen; ++nIndex)
        {
            CommandData.Data.ReadUserDataWithDelay[nIndex].nAddress = pReadUserDataWithDelay[nReadUserDataWithDelayIndex + nIndex].nAddress;
            CommandData.Data.ReadUserDataWithDelay[nIndex].nDelayNanoSeconds = pReadUserDataWithDelay[nReadUserDataWithDelayIndex + nIndex].nDelayNanoSeconds;
            CommandData.Data.ReadUserDataWithDelay[nIndex].nPerformRead = (UINT8)pReadUserDataWithDelay[nReadUserDataWithDelayIndex + nIndex].bPerformRead;
        }

        if (lSendDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefSendTimeoutMs, &CommandData) &&
            lReceiveDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefReceiveTimeoutMs, &ResponseData))
        {
            if (ResponseData.ErrorCode == CPepErrorSuccess)
            {
                for (nIndex = 0; nIndex < CommandData.Data.nReadUserDataWithDelayLen; ++nIndex)
                {
                    if (CommandData.Data.ReadUserDataWithDelay[nIndex].nPerformRead)
                    {
                        *(pbyData + nDataIndex) = ResponseData.Data.nData[nIndex];

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
    TUtPepCommandData CommandData;
    TUtPepResponseData ResponseData;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    CommandData.Command = CPepProgramDataCommand;
    CommandData.Data.ProgramData.nAddress = nAddress;

    while (bResult && nIndex < nDataLen)
    {
        nTotal = nDataLen - nIndex;

        if (nTotal > MArrayLen(CommandData.Data.ProgramData.nData))
        {
            nTotal = MArrayLen(CommandData.Data.ProgramData.nData);
        }

        CommandData.Data.ProgramData.nDataLen = (UINT8)nTotal;

        CopyMemory(CommandData.Data.ProgramData.nData, pbyData + nIndex, CommandData.Data.ProgramData.nDataLen);

        if (lSendDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefSendTimeoutMs, &CommandData) &&
            lReceiveDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefReceiveTimeoutMs, &ResponseData))
        {
            if (ResponseData.ErrorCode == CPepErrorSuccess)
            {
                CommandData.Data.ProgramData.nAddress += CommandData.Data.ProgramData.nDataLen;

                nIndex += CommandData.Data.ProgramData.nDataLen;
            }
            else
            {
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
    TUtPepCommandData CommandData;
    TUtPepResponseData ResponseData;

    nDataLen;

    WaitForSingleObject(l_DeviceData.hDeviceMutex, INFINITE);

    CommandData.Command = CPepProgramUserDataCommand;

    while (bResult && nProgramUserDataIndex < nProgramUserDataLen)
    {
        nTotal = nProgramUserDataLen - nProgramUserDataIndex;

        if (nTotal > MArrayLen(CommandData.Data.ProgramUserData))
        {
            nTotal = MArrayLen(CommandData.Data.ProgramUserData);
        }

        CommandData.Data.nProgramUserDataLen = (UINT8)nTotal;

        for (nIndex = 0; nIndex < CommandData.Data.nProgramUserDataLen; ++nIndex)
        {
            CommandData.Data.ProgramUserData[nIndex].nAddress = pProgramUserData[nProgramUserDataIndex + nIndex].nAddress;
            CommandData.Data.ProgramUserData[nIndex].nPerformProgram = (UINT8)pProgramUserData[nProgramUserDataIndex + nIndex].bPerformProgram;

            if (CommandData.Data.ProgramUserData[nIndex].nPerformProgram)
            {
                CommandData.Data.ProgramUserData[nIndex].nData = *(pbyData + nDataIndex);

                ++nDataIndex;
            }
        }

        if (lSendDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefSendTimeoutMs, &CommandData) &&
            lReceiveDeviceCommand(l_DeviceData.UsbDeviceData.hDevice, CDefReceiveTimeoutMs, &ResponseData))
        {
            if (ResponseData.ErrorCode == CPepErrorSuccess)
            {
                nProgramUserDataIndex += CommandData.Data.nProgramUserDataLen;
            }
            else
            {
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
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/
