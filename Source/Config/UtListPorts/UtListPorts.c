/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>

#include <setupapi.h>

#include <ntddpar.h>

#include "UsbPrintGuid.h"

#include <Config/UtListPorts.h>

#include <Utils/UtHeap.h>

#define CDeviceParallelPortPrefix L"\\Device\\ParallelPort"
#define CDeviceParallelPortPrefixLen 20

typedef struct tagTDeviceInfo
{
    LPWSTR pszLocation;
    LPWSTR pszPhysicalDeviceObjectName;
} TDeviceInfo;

static TDeviceInfo* l_pLptPortDeviceInfo = NULL;
static INT l_nLptPortDeviceInfoCount = 0;

static TDeviceInfo* l_pUsbPrintPortDeviceInfo = NULL;
static INT l_nUsbPrintPortDeviceInfoCount = 0;

static BOOL lGetPortData(
  TDeviceInfo* pDeviceInfo,
  INT nPortData,
  LPWSTR pszData,
  LPINT pnDataLen)
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
  TDeviceInfo* pDeviceInfo,
  INT nDeviceInfoCount)
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
  HDEVINFO hDevInfo,
  PSP_DEVINFO_DATA pDevInfoData,
  DWORD dwProperty,
  LPWSTR* ppszValue)
{
    DWORD dwDataType, dwBufferLen;

    *ppszValue = NULL;

    if (!SetupDiGetDeviceRegistryProperty(hDevInfo, pDevInfoData, dwProperty,
                                          &dwDataType, NULL, 0, &dwBufferLen) &&
        GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
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
  LPGUID pInterfaceGuid,
  TDeviceInfo** ppDeviceInfo,
  LPINT pnDeviceInfoCount)
{
    HDEVINFO hDevInfo;
    DWORD dwDevCount, dwBufferLen;
    SP_DEVINFO_DATA DevInfoData;
    SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData;
    TDeviceInfo* pDeviceInfo;

    *ppDeviceInfo = NULL;
    *pnDeviceInfoCount = 0;

    hDevInfo = SetupDiGetClassDevs(pInterfaceGuid, 0, 0,
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

static VOID lFixParallelPortDeviceInfo(
  TDeviceInfo* pDeviceInfo,
  INT nDeviceInfoCount)
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
    GUID InterfaceGuid;

    if (l_pLptPortDeviceInfo || l_pUsbPrintPortDeviceInfo)
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

    if (UtUninitHeap() == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL UTLISTPORTSAPI UtListPortsGetLptPortCount(
  LPINT pnCount)
{
    *pnCount = l_nLptPortDeviceInfoCount;

    return TRUE;
}

BOOL UTLISTPORTSAPI UtListPortsGetLptPortData(
  INT nIndex,
  INT nPortData,
  LPWSTR pszData,
  LPINT pnDataLen)
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
  LPINT pnCount)
{
    *pnCount = l_nUsbPrintPortDeviceInfoCount;

    return TRUE;
}

BOOL UTLISTPORTSAPI UtListPortsGetUsbPrintPortData(
  INT nIndex,
  INT nPortData,
  LPWSTR pszData,
  LPINT pnDataLen)
{
    TDeviceInfo* pDeviceInfo;

    if (nIndex >= l_nUsbPrintPortDeviceInfoCount)
    {
        return FALSE;
    }

    pDeviceInfo = &l_pUsbPrintPortDeviceInfo[nIndex];

    return lGetPortData(pDeviceInfo, nPortData, pszData, pnDataLen);
}

/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/
