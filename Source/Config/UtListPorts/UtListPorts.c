/***************************************************************************/
/*  Copyright (C) 2007-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>

#include <setupapi.h>

#include <ntddpar.h>
#include <hidsdi.h>

#include "UsbPrintGuid.h"

#include <Config/UtListPorts.h>

#include <Utils/UtHeap.h>

#include <Firmware/PepFirmwareDefs.h>

#define CDeviceParallelPortPrefix L"\\Device\\ParallelPort"
#define CDeviceParallelPortPrefixLen 20

#define CDeviceUsbPortLocation L"PEP to EPROM+ Adapter #"
#define CDeviceUsbPortLocationLen 23

typedef struct tagTDeviceInfo
{
    LPWSTR pszLocation;
    LPWSTR pszPhysicalDeviceObjectName;
} TDeviceInfo;

static TDeviceInfo* l_pLptPortDeviceInfo = NULL;
static INT l_nLptPortDeviceInfoCount = 0;

static TDeviceInfo* l_pUsbPrintPortDeviceInfo = NULL;
static INT l_nUsbPrintPortDeviceInfoCount = 0;

static TDeviceInfo* l_pUsbHIDPortDeviceInfo = NULL;
static INT l_nUsbHIDPortDeviceInfoCount = 0;

static TDeviceInfo* l_pUsbPortDeviceInfo = NULL;
static INT l_nUsbPortDeviceInfoCount = 0;

static BOOL lGetPortData(
  _In_ TDeviceInfo* pDeviceInfo,
  _In_ INT nPortData,
  _Out_writes_z_(*pnDataLen) LPWSTR pszData,
  _Out_writes_bytes_(sizeof(INT)) LPINT pnDataLen)
{
    LPCWSTR pszPortData;
    INT nPortDataLen;

    switch (nPortData)
    {
        case epdLocation:
            pszPortData = pDeviceInfo->pszLocation;
            break;
        case epdPhysicalDeviceObjectName:
            pszPortData = pDeviceInfo->pszPhysicalDeviceObjectName;
            break;
        default:
            return FALSE;
    }

    if (pszPortData)
    {
        nPortDataLen = (lstrlenW(pszPortData) + 1) * sizeof(WCHAR);

        if (pszData)
        {
            if (*pnDataLen < nPortDataLen)
            {
                return FALSE;
            }

            StringCchCopyW(pszData, *pnDataLen, pszPortData);
        }
        else
        {
            *pnDataLen = nPortDataLen;
        }
    }
    else
    {
        *pnDataLen = 0;

        return FALSE;
    }

    return TRUE;
}

static VOID lFreeDeviceInfos(
  _In_ TDeviceInfo* pDeviceInfo,
  _In_ INT nDeviceInfoCount)
{
    INT nIndex;

    for (nIndex = 0; nIndex < nDeviceInfoCount; ++nIndex)
    {
        if (pDeviceInfo[nIndex].pszLocation)
        {
            UtFreeMem(pDeviceInfo[nIndex].pszLocation);
        }

        if (pDeviceInfo[nIndex].pszPhysicalDeviceObjectName)
        {
            UtFreeMem(pDeviceInfo[nIndex].pszPhysicalDeviceObjectName);
        }
    }

    UtFreeMem(pDeviceInfo);
}

BOOL lReadDeviceRegPropString(
  _In_ HDEVINFO hDevInfo,
  _In_ PSP_DEVINFO_DATA pDevInfoData,
  _In_ DWORD dwProperty,
  _Out_ LPWSTR* ppszValue)
{
    DWORD dwDataType, dwBufferLen;

    *ppszValue = NULL;

    if (!SetupDiGetDeviceRegistryProperty(hDevInfo, pDevInfoData, dwProperty,
                                          &dwDataType, NULL, 0, &dwBufferLen) &&
        GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        if (GetLastError() == ERROR_INVALID_DATA)
        {
            *ppszValue = (LPWSTR)UtAllocMem(1);

            **ppszValue = 0;

            return TRUE;
        }

        return FALSE;
    }

    if (dwDataType != REG_SZ)
    {
        return FALSE;
    }

    *ppszValue = (LPWSTR)UtAllocMem(dwBufferLen);

    if (!SetupDiGetDeviceRegistryProperty(hDevInfo, pDevInfoData, dwProperty,
                                          NULL, (LPBYTE)*ppszValue, dwBufferLen,
                                          NULL))
    {
        UtFreeMem(ppszValue);

        return FALSE;
    }

    return TRUE;
}

static BOOL lEnumDeviceInterface(
  _In_ LPGUID pInterfaceGuid,
  _Out_ TDeviceInfo** ppDeviceInfo,
  _Out_writes_bytes_(sizeof(INT)) LPINT pnDeviceInfoCount)
{
    HDEVINFO hDevInfo;
    DWORD dwDevCount, dwBufferLen;
    SP_DEVINFO_DATA DevInfoData;
    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData;
    TDeviceInfo* pDeviceInfo;

    *ppDeviceInfo = NULL;
    *pnDeviceInfoCount = 0;

    hDevInfo = SetupDiGetClassDevs(pInterfaceGuid, NULL, NULL,
                                   DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    dwDevCount = 0;

    DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);

    while (SetupDiEnumDeviceInterfaces(hDevInfo, 0, pInterfaceGuid,
                                       dwDevCount, &DeviceInterfaceData))
    {
        if (*pnDeviceInfoCount > 0)
        {
            *ppDeviceInfo = (TDeviceInfo*)UtReAllocMem(*ppDeviceInfo,
                                                       (*pnDeviceInfoCount + 1) * sizeof(TDeviceInfo));
        }
        else
        {
            *ppDeviceInfo = (TDeviceInfo*)UtAllocMem(sizeof(TDeviceInfo));
        }

        pDeviceInfo = *ppDeviceInfo + *pnDeviceInfoCount;

        ZeroMemory(pDeviceInfo, sizeof(TDeviceInfo));

        *pnDeviceInfoCount += 1;

        dwBufferLen = 0;

        SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, 0, 0,
                                        &dwBufferLen, 0);

        DevInfoData.cbSize = sizeof(DevInfoData);

        pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)UtAllocMem(dwBufferLen);

        pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        DevInfoData.cbSize = sizeof(DevInfoData);

        if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData,
                                             pDeviceInterfaceDetailData,
                                             dwBufferLen, 0, &DevInfoData))
        {
            UtFreeMem(pDeviceInterfaceDetailData);

            SetupDiDestroyDeviceInfoList(hDevInfo);

            lFreeDeviceInfos(*ppDeviceInfo, *pnDeviceInfoCount);

            *ppDeviceInfo = NULL;
            *pnDeviceInfoCount = 0;

            return FALSE;
        }

        dwBufferLen = (lstrlenW(pDeviceInterfaceDetailData->DevicePath) + 1) * sizeof(WCHAR);

        UtFreeMem(pDeviceInterfaceDetailData);

        if ((FALSE == lReadDeviceRegPropString(hDevInfo, &DevInfoData,
                                               SPDRP_FRIENDLYNAME,
                                               &pDeviceInfo->pszLocation) &&
             FALSE == lReadDeviceRegPropString(hDevInfo, &DevInfoData,
                                               SPDRP_LOCATION_INFORMATION,
                                               &pDeviceInfo->pszLocation)) ||
             FALSE == lReadDeviceRegPropString(hDevInfo, &DevInfoData,
                                               SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
                                               &pDeviceInfo->pszPhysicalDeviceObjectName))
        {
            if (pDeviceInfo->pszLocation)
            {
                UtFreeMem(pDeviceInfo->pszLocation);
            }

            if (*pnDeviceInfoCount == 1)
            {
                UtFreeMem(*ppDeviceInfo);

                *ppDeviceInfo = NULL;
            }

            *pnDeviceInfoCount -= 1;
        }

        ++dwDevCount;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    return TRUE;
}

static BOOL lIsUsbHidDevice(
  _In_z_ LPCWSTR pszDevicePath)
{
    HANDLE hDevice;
    HIDD_ATTRIBUTES Attributes;
    BOOL bResult;

    hDevice = CreateFile(pszDevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                         NULL, OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    bResult = FALSE;

    Attributes.Size = sizeof(Attributes);

    if (HidD_GetAttributes(hDevice, &Attributes))
    {
        if (Attributes.VendorID == CMicrochipFirmwareVendorID &&
            Attributes.ProductID == CMicrochipHIDFirmwareProductID)
        {
            bResult = TRUE;
        }
    }

    CloseHandle(hDevice);

    return bResult;
}

static BOOL lEnumUsbHidDeviceInterface(
  _In_ LPGUID pInterfaceGuid,
  _Out_ TDeviceInfo** ppDeviceInfo,
  _Out_writes_bytes_(sizeof(INT)) LPINT pnDeviceInfoCount)
{
    HDEVINFO hDevInfo;
    DWORD dwDevCount, dwBufferLen, dwRequiredLen;
    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData;
    TDeviceInfo* pDeviceInfo;

    *ppDeviceInfo = NULL;
    *pnDeviceInfoCount = 0;

    hDevInfo = SetupDiGetClassDevs(pInterfaceGuid, NULL, NULL,
                                   DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    dwDevCount = 0;

    DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);

    while (SetupDiEnumDeviceInterfaces(hDevInfo, 0, pInterfaceGuid,
                                       dwDevCount, &DeviceInterfaceData))
    {
        dwBufferLen = 0;

        SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData, 0, 0,
                                        &dwBufferLen, 0);

        pDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)UtAllocMem(dwBufferLen);

        pDeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &DeviceInterfaceData,
                                             pDeviceInterfaceDetailData,
                                             dwBufferLen, &dwRequiredLen, NULL))
        {
            UtFreeMem(pDeviceInterfaceDetailData);

            SetupDiDestroyDeviceInfoList(hDevInfo);

            lFreeDeviceInfos(*ppDeviceInfo, *pnDeviceInfoCount);

            *ppDeviceInfo = NULL;
            *pnDeviceInfoCount = 0;

            return FALSE;
        }

        if (lIsUsbHidDevice(pDeviceInterfaceDetailData->DevicePath))
        {
            if (*pnDeviceInfoCount > 0)
            {
                *ppDeviceInfo = (TDeviceInfo*)UtReAllocMem(*ppDeviceInfo,
                                                           (*pnDeviceInfoCount + 1) * sizeof(TDeviceInfo));
            }
            else
            {
                *ppDeviceInfo = (TDeviceInfo*)UtAllocMem(sizeof(TDeviceInfo));
            }

            pDeviceInfo = *ppDeviceInfo + *pnDeviceInfoCount;

            ZeroMemory(pDeviceInfo, sizeof(TDeviceInfo));

            *pnDeviceInfoCount += 1;

            dwBufferLen = (lstrlenW(pDeviceInterfaceDetailData->DevicePath) + 1) * sizeof(WCHAR);

            pDeviceInfo->pszPhysicalDeviceObjectName = (LPWSTR)UtAllocMem(dwBufferLen);

            StringCbCopyW(pDeviceInfo->pszPhysicalDeviceObjectName, dwBufferLen,
                          pDeviceInterfaceDetailData->DevicePath);

            dwBufferLen = (CDeviceUsbPortLocationLen + 5) * sizeof(WCHAR);

            pDeviceInfo->pszLocation = (LPWSTR)UtAllocMem(dwBufferLen);

            StringCbPrintf(pDeviceInfo->pszLocation, dwBufferLen, L"%s%d",
                           CDeviceUsbPortLocation, *pnDeviceInfoCount);
        }

        UtFreeMem(pDeviceInterfaceDetailData);

        ++dwDevCount;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);

    return TRUE;
}

static VOID lFixParallelPortDeviceInfo(
  _In_ TDeviceInfo* pDeviceInfo,
  _In_ INT nDeviceInfoCount)
{
    TDeviceInfo* pCurDeviceInfo;
    INT nIndex, nLen;
    WCHAR cPortNum;

    for (nIndex = 0; nIndex < nDeviceInfoCount; ++nIndex)
    {
        pCurDeviceInfo = pDeviceInfo + nIndex;

        cPortNum = pCurDeviceInfo->pszLocation[lstrlenW(pCurDeviceInfo->pszLocation) - 2];
        nLen = CDeviceParallelPortPrefixLen + 2;

        pCurDeviceInfo->pszPhysicalDeviceObjectName =
            (LPWSTR)UtReAllocMem(pCurDeviceInfo->pszPhysicalDeviceObjectName,
                                 nLen * sizeof(WCHAR));

        StringCchCopyW(pCurDeviceInfo->pszPhysicalDeviceObjectName, nLen,
                       CDeviceParallelPortPrefix);

        pCurDeviceInfo->pszPhysicalDeviceObjectName[nLen - 2] = cPortNum - 1;
        pCurDeviceInfo->pszPhysicalDeviceObjectName[nLen - 1] = 0;
    }
}

BOOL UTLISTPORTSAPI UtListPortsInitialize(VOID)
{
    WCHAR cPepFirmwareDeviceInterfaceGuid[] = { CPepFirmwareDeviceInterfaceGuid, 0 };

    GUID InterfaceGuid;

    if (l_pLptPortDeviceInfo ||
        l_pUsbPrintPortDeviceInfo ||
        l_pUsbHIDPortDeviceInfo ||
        l_pUsbPortDeviceInfo)
    {
        return TRUE;
    }

    if (UtInitHeap() == FALSE)
    {
        return FALSE;
    }

    InterfaceGuid = GUID_DEVINTERFACE_PARALLEL;

    if (!lEnumDeviceInterface(&InterfaceGuid, &l_pLptPortDeviceInfo,
                              &l_nLptPortDeviceInfoCount))
    {
        UtListPortsUninitialize();

        return FALSE;
    }

    lFixParallelPortDeviceInfo(l_pLptPortDeviceInfo, l_nLptPortDeviceInfoCount);

    InterfaceGuid = GUID_DEVINTERFACE_USBPRINT;

    if (!lEnumDeviceInterface(&InterfaceGuid, &l_pUsbPrintPortDeviceInfo,
                              &l_nUsbPrintPortDeviceInfoCount))
    {
        UtListPortsUninitialize();

        return FALSE;
    }

    HidD_GetHidGuid(&InterfaceGuid);

    if (!lEnumUsbHidDeviceInterface(&InterfaceGuid, &l_pUsbHIDPortDeviceInfo,
                                    &l_nUsbHIDPortDeviceInfoCount))
    {
        UtListPortsUninitialize();

        return FALSE;
    }

    if (S_OK != UuidFromStringW(cPepFirmwareDeviceInterfaceGuid, &InterfaceGuid))
    {
        UtListPortsUninitialize();

        return FALSE;
    }

    if (!lEnumDeviceInterface(&InterfaceGuid, &l_pUsbPortDeviceInfo,
                              &l_nUsbPortDeviceInfoCount))
    {
        UtListPortsUninitialize();

        return FALSE;
    }

    return TRUE;
}

BOOL UTLISTPORTSAPI UtListPortsUninitialize(VOID)
{
    if (l_pLptPortDeviceInfo)
    {
        lFreeDeviceInfos(l_pLptPortDeviceInfo, l_nLptPortDeviceInfoCount);

        l_pLptPortDeviceInfo = NULL;
        l_nLptPortDeviceInfoCount = 0;
    }

    if (l_pUsbPrintPortDeviceInfo)
    {
        lFreeDeviceInfos(l_pUsbPrintPortDeviceInfo, l_nUsbPrintPortDeviceInfoCount);

        l_pUsbPrintPortDeviceInfo = NULL;
        l_nUsbPrintPortDeviceInfoCount = 0;
    }

    if (l_pUsbHIDPortDeviceInfo)
    {
        lFreeDeviceInfos(l_pUsbHIDPortDeviceInfo, l_nUsbHIDPortDeviceInfoCount);

        l_pUsbHIDPortDeviceInfo = NULL;
        l_nUsbHIDPortDeviceInfoCount = 0;
    }

    if (l_pUsbPortDeviceInfo)
    {
        lFreeDeviceInfos(l_pUsbPortDeviceInfo, l_nUsbPortDeviceInfoCount);

        l_pUsbPortDeviceInfo = NULL;
        l_nUsbPortDeviceInfoCount = 0;
    }

    if (UtUninitHeap() == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL UTLISTPORTSAPI UtListPortsGetLptPortCount(
  _Out_writes_bytes_(sizeof(INT)) LPINT pnCount)
{
    *pnCount = l_nLptPortDeviceInfoCount;

    return TRUE;
}

BOOL UTLISTPORTSAPI UtListPortsGetLptPortData(
  _In_ INT nIndex,
  _In_ INT nPortData,
  _Out_writes_z_(*pnDataLen) LPWSTR pszData,
  _Out_writes_bytes_(sizeof(INT)) LPINT pnDataLen)
{
    TDeviceInfo* pDeviceInfo;

    if (nIndex >= l_nLptPortDeviceInfoCount)
    {
        return FALSE;
    }

    pDeviceInfo = &l_pLptPortDeviceInfo[nIndex];

    return lGetPortData(pDeviceInfo, nPortData, pszData, pnDataLen);
}

BOOL UTLISTPORTSAPI UtListPortsGetUsbPrintPortCount(
  _Out_writes_bytes_(sizeof(INT)) LPINT pnCount)
{
    *pnCount = l_nUsbPrintPortDeviceInfoCount;

    return TRUE;
}

BOOL UTLISTPORTSAPI UtListPortsGetUsbPrintPortData(
  _In_ INT nIndex,
  _In_ INT nPortData,
  _Out_writes_z_(*pnDataLen) LPWSTR pszData,
  _Out_writes_bytes_(sizeof(INT)) LPINT pnDataLen)
{
    TDeviceInfo* pDeviceInfo;

    if (nIndex >= l_nUsbPrintPortDeviceInfoCount)
    {
        return FALSE;
    }

    pDeviceInfo = &l_pUsbPrintPortDeviceInfo[nIndex];

    return lGetPortData(pDeviceInfo, nPortData, pszData, pnDataLen);
}

BOOL UTLISTPORTSAPI UtListPortsGetUsbHIDPortCount(
  _Out_writes_bytes_(sizeof(INT)) LPINT pnCount)
{
    *pnCount = l_nUsbHIDPortDeviceInfoCount;

    return TRUE;
}

BOOL UTLISTPORTSAPI UtListPortsGetUsbHIDPortData(
  _In_ INT nIndex,
  _In_ INT nPortData,
  _Out_writes_z_(*pnDataLen) LPWSTR pszData,
  _Out_writes_bytes_(sizeof(INT)) LPINT pnDataLen)
{
    TDeviceInfo* pDeviceInfo;

    if (nIndex >= l_nUsbHIDPortDeviceInfoCount)
    {
        return FALSE;
    }

    pDeviceInfo = &l_pUsbHIDPortDeviceInfo[nIndex];

    return lGetPortData(pDeviceInfo, nPortData, pszData, pnDataLen);
}

BOOL UTLISTPORTSAPI UtListPortsGetUsbPortCount(
  _Out_writes_bytes_(sizeof(INT)) LPINT pnCount)
{
    *pnCount = l_nUsbPortDeviceInfoCount;

    return TRUE;
}

BOOL UTLISTPORTSAPI UtListPortsGetUsbPortData(
  _In_ INT nIndex,
  _In_ INT nPortData,
  _Out_writes_z_(*pnDataLen) LPWSTR pszData,
  _Out_writes_bytes_(sizeof(INT)) LPINT pnDataLen)
{
    TDeviceInfo* pDeviceInfo;

    if (nIndex >= l_nUsbPortDeviceInfoCount)
    {
        return FALSE;
    }

    pDeviceInfo = &l_pUsbPortDeviceInfo[nIndex];

    return lGetPortData(pDeviceInfo, nPortData, pszData, pnDataLen);
}

/***************************************************************************/
/*  Copyright (C) 2007-2021 Kevin Eshbach                                  */
/***************************************************************************/
