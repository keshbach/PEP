/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>
#include <assert.h>

#include <UtilsDevice/UtPepDevices.h>
#include <UtilsDevice/UtPAL.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeap.h>

/* Pin Diagram definitions */

#define CPinDiagramEOL L"\r\n"

#define CDipPinDiagramTopRow L"   |----\\_/----|"
#define CDipPinDiagramBottomRow L"   |-----------|"

#define CDipPinDiagramRowFormat L" --| %-2d     %2d |-- "

/* Function definitions */

typedef VOID (UTPEPDEVICESAPI *TGetDeviceCountFunc)(PULONG pulDeviceCount);
typedef BOOL (UTPEPDEVICESAPI *TGetDeviceFunc)(ULONG ulIndex, TDevice* pDevice);

typedef LPWSTR (WINAPI* TPathFindFileNameFunc)(LPCWSTR pszPath);

typedef DWORD (WINAPI* TGetFileVersionInfoSizeFunc)(LPCWSTR lpszFilename, LPDWORD lpdwHandle);
typedef BOOL  (WINAPI* TGetFileVersionInfoFunc)(LPCWSTR lpszFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
typedef BOOL  (WINAPI* TVerQueryValueFunc)(const LPVOID pvBlock, LPWSTR pszSubBlock, LPVOID* ppvBuffer, PUINT puLen);

typedef struct tagTPlugin
{
    HMODULE hModule;
    LPWSTR pszName;
    WORD wProductMajorVersion;
    WORD wProductMinorVersion;
    WORD wProductBuildVersion;
    WORD wProductPrivateVersion;
    TDevice* pDevices;
    ULONG ulDeviceCount;
} TPlugin;

static BOOL l_bInitialize = TRUE;
static TPlugin* l_pPlugins = NULL;
static ULONG l_ulPluginCount = 0;

static BOOL lInitPluginFileName(
  TPlugin* pPlugin,
  LPCWSTR pszPlugin)
{
    BOOL bResult = FALSE;
    HMODULE hModule = LoadLibraryW(L"shlwapi.dll");
    TPathFindFileNameFunc pPathFindFileNameFunc;
    LPWSTR pszName;
    INT nNameLen;

    if (hModule == NULL)
    {
        goto End;
    }

    pPathFindFileNameFunc = (TPathFindFileNameFunc)GetProcAddress(hModule, "PathFindFileNameW");

    if (pPathFindFileNameFunc == NULL)
    {
        goto End;
    }

    pszName = pPathFindFileNameFunc(pszPlugin);
    nNameLen = lstrlenW(pszName) + 1;

    pPlugin->pszName = (LPWSTR)UtAllocMem(nNameLen * sizeof(WCHAR));

    StringCchCopyW(pPlugin->pszName, nNameLen, pszName);

    bResult = TRUE;

End:
    if (hModule)
    {
        FreeLibrary(hModule);
    }

    return bResult;
}

static BOOL lInitPluginFileVersion(
  TPlugin* pPlugin,
  LPCWSTR pszPlugin)
{
    BOOL bResult = FALSE;
    HMODULE hModule = LoadLibraryW(L"version.dll");
    TGetFileVersionInfoSizeFunc pGetFileVersionInfoSizeFunc;
    TGetFileVersionInfoFunc pGetFileVersionInfoFunc;
    TVerQueryValueFunc pVerQueryValueFunc;
    LPVOID pvData;
    VS_FIXEDFILEINFO* pFixedFileInfo;
    DWORD dwVersionLen, dwHandle;
    UINT uiLen;

    if (hModule == NULL)
    {
        goto End;
    }

    pGetFileVersionInfoSizeFunc = (TGetFileVersionInfoSizeFunc)GetProcAddress(hModule, "GetFileVersionInfoSizeW");
    pGetFileVersionInfoFunc = (TGetFileVersionInfoFunc)GetProcAddress(hModule, "GetFileVersionInfoW");
    pVerQueryValueFunc = (TVerQueryValueFunc)GetProcAddress(hModule, "VerQueryValueW");

    if (pGetFileVersionInfoSizeFunc == NULL ||
        pGetFileVersionInfoFunc == NULL ||
        pVerQueryValueFunc == NULL)
    {
        goto End;
    }

    dwVersionLen = pGetFileVersionInfoSizeFunc(pszPlugin, &dwHandle);

    if (dwVersionLen == 0)
    {
        goto End;
    }

    pvData = UtAllocMem(dwVersionLen);

    if (pGetFileVersionInfoFunc(pszPlugin, dwHandle, dwVersionLen, pvData) &&
        pVerQueryValueFunc(pvData, TEXT("\\"), (LPVOID*)&pFixedFileInfo, &uiLen))
    {
        pPlugin->wProductMajorVersion   = HIWORD(pFixedFileInfo->dwProductVersionMS);
        pPlugin->wProductMinorVersion   = LOWORD(pFixedFileInfo->dwProductVersionMS);
        pPlugin->wProductBuildVersion   = HIWORD(pFixedFileInfo->dwProductVersionLS);
        pPlugin->wProductPrivateVersion = LOWORD(pFixedFileInfo->dwProductVersionLS);

        bResult = TRUE;
    }

    UtFreeMem(pvData);

End:
    if (hModule)
    {
        FreeLibrary(hModule);
    }

    return bResult;
}

static BOOL lLoadDevicePlugin(
  LPCWSTR pszPlugin)
{
    HMODULE hModule = LoadLibraryW(pszPlugin);
    TGetDeviceCountFunc pGetDeviceCountFunc;
    TGetDeviceFunc pGetDeviceFunc;
    TPlugin* pPlugin;
    ULONG ulDeviceCount, ulIndex;
    TDevice Device;
    TDevice* pDevice;

    if (hModule == NULL)
    {
        return FALSE;
    }

    pGetDeviceCountFunc = (TGetDeviceCountFunc)GetProcAddress(hModule, "GetDeviceCount");
    pGetDeviceFunc = (TGetDeviceFunc)GetProcAddress(hModule, "GetDevice");

    if (pGetDeviceCountFunc == NULL || pGetDeviceFunc == NULL)
    {
        FreeLibrary(hModule);

        return FALSE;
    }

    ++l_ulPluginCount;

    if (l_pPlugins)
    {
        l_pPlugins = (TPlugin*)UtReAllocMem(l_pPlugins,
                                           l_ulPluginCount * sizeof(TPlugin));
    }
    else
    {
        l_pPlugins = (TPlugin*)UtAllocMem(sizeof(TPlugin));
    }

    pPlugin = &l_pPlugins[l_ulPluginCount - 1];

    ZeroMemory(pPlugin, sizeof(TPlugin));

    pPlugin->hModule = hModule;

    if (FALSE == lInitPluginFileName(pPlugin, pszPlugin) ||
        FALSE == lInitPluginFileVersion(pPlugin, pszPlugin))
    {
        if (pPlugin->pszName)
        {
            UtFreeMem(pPlugin->pszName);                
        }

        FreeLibrary(hModule);

        --l_ulPluginCount;

        return FALSE;
    }

    pGetDeviceCountFunc(&ulDeviceCount);

    for (ulIndex = 0; ulIndex < ulDeviceCount; ++ulIndex)
    {
        if (TRUE == pGetDeviceFunc(ulIndex, &Device) && Device.pszName)
        {
            if (Device.pszAdapter == NULL)
            {
                Device.pszAdapter = L"";
            }

            pPlugin->ulDeviceCount += 1;

            if (pPlugin->pDevices)
            {
                pPlugin->pDevices = (TDevice*)UtReAllocMem(pPlugin->pDevices,
                                        pPlugin->ulDeviceCount * sizeof(TDevice));
            }
            else
            {
                pPlugin->pDevices = (TDevice*)UtAllocMem(sizeof(TDevice));
            }

            pDevice = &pPlugin->pDevices[pPlugin->ulDeviceCount - 1];

            CopyMemory(pDevice, &Device, sizeof(Device));
        }
    }

    return TRUE;
}

static LPCWSTR lAllocDipPackagePinDiagram(
  LPCWSTR* ppszPinNames,
  UINT nPinNamesCount)
{
    UINT nTotalLeftColumns, nIndex, nIndex2, nTmpLen, nPinDiagramLen;
    LPWSTR pszPinDiagram;
    WCHAR cBuffer[30];

    if (nPinNamesCount > 99)
    {
        /* Function hardcoded to only support two digits. */

        return NULL;
    }

    nTotalLeftColumns = 0;
    nPinDiagramLen = 0;

    for (nIndex = 0; nIndex < nPinNamesCount / 2; ++nIndex)
    {
        nTmpLen = lstrlenW(ppszPinNames[nIndex]);

        if (nTmpLen > nTotalLeftColumns)
        {
            nTotalLeftColumns = nTmpLen;
        }

        nPinDiagramLen += lstrlenW(ppszPinNames[nPinNamesCount - (nIndex + 1)]);
    }

    nPinDiagramLen += lstrlenW(CDipPinDiagramTopRow) +
                      lstrlenW(CDipPinDiagramBottomRow);
    nPinDiagramLen += (nTotalLeftColumns * ((nPinNamesCount / 2) + 2));
    nPinDiagramLen += (lstrlenW(CDipPinDiagramRowFormat) * (nPinNamesCount / 2));
    nPinDiagramLen += (lstrlenW(CPinDiagramEOL) * ((nPinNamesCount / 2) + 2));

    pszPinDiagram = (LPWSTR)UtAllocMem(nPinDiagramLen * sizeof(WCHAR));

    if (pszPinDiagram == NULL)
    {
        return NULL;
    }

    *pszPinDiagram = 0;
    
    // Build the first row

    for (nIndex = 0; nIndex < nTotalLeftColumns; ++nIndex)
    {
        StringCchCatW(pszPinDiagram, nPinDiagramLen, L" ");
    }

    StringCchCatW(pszPinDiagram, nPinDiagramLen, CDipPinDiagramTopRow);
    StringCchCatW(pszPinDiagram, nPinDiagramLen, CPinDiagramEOL);

    // Build the pin rows

    for (nIndex = 0; nIndex < nPinNamesCount / 2; ++nIndex)
    {
        StringCchPrintfW(cBuffer, sizeof(cBuffer) / sizeof(cBuffer[0]),
                         CDipPinDiagramRowFormat,
                         nIndex + 1, nPinNamesCount - nIndex);

        StringCchCatW(pszPinDiagram, nPinDiagramLen, ppszPinNames[nIndex]);

        for (nIndex2 = lstrlenW(ppszPinNames[nIndex]);
             nIndex2 < nTotalLeftColumns; ++nIndex2)
        {
            StringCchCatW(pszPinDiagram, nPinDiagramLen, L" ");
        }

        StringCchCatW(pszPinDiagram, nPinDiagramLen, cBuffer);
        StringCchCatW(pszPinDiagram, nPinDiagramLen,
                      ppszPinNames[nPinNamesCount - (nIndex + 1)]);
        StringCchCatW(pszPinDiagram, nPinDiagramLen, CPinDiagramEOL);
    }

    // Build the last row

    for (nIndex = 0; nIndex < nTotalLeftColumns; ++nIndex)
    {
        StringCchCatW(pszPinDiagram, nPinDiagramLen, L" ");
    }
    
    StringCchCatW(pszPinDiagram, nPinDiagramLen, CDipPinDiagramBottomRow);
    StringCchCatW(pszPinDiagram, nPinDiagramLen, CPinDiagramEOL);

    return pszPinDiagram;
}

static LPCWSTR lAllocPlccPackagePinDiagram(
  LPCWSTR* ppszPinNames,
  UINT nPinNamesCount)
{
	ppszPinNames;
	nPinNamesCount;

	return NULL;
}

BOOL UTPEPDEVICESAPI UtPepDevicesInitialize(
  LPCWSTR pszPluginPath)
{
    WCHAR cPath[MAX_PATH];
    HANDLE hFindFile;
    WIN32_FIND_DATAW FindFileData;

    if (l_bInitialize == FALSE)
    {
        return TRUE;
    }

    if (UtInitHeap() == FALSE)
    {
        return FALSE;
    }

    l_bInitialize = TRUE;

    StringCchCopyW(cPath, MArrayLen(cPath), pszPluginPath);
    StringCchCatW(cPath, MArrayLen(cPath), L"\\*.dll");

    hFindFile = FindFirstFileW(cPath, &FindFileData);

    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    do
    {
        StringCchCopyW(cPath, MArrayLen(cPath), pszPluginPath);
        StringCchCatW(cPath, MArrayLen(cPath), L"\\");
        StringCchCatW(cPath, MArrayLen(cPath), FindFileData.cFileName);

        lLoadDevicePlugin(cPath);
    } while (FindNextFile(hFindFile, &FindFileData));

    FindClose(hFindFile);

    return TRUE;
}

BOOL UTPEPDEVICESAPI UtPepDevicesUninitialize(VOID)
{
    TPlugin* pPlugin;
    ULONG ulIndex;

    if (l_bInitialize == FALSE)
    {
        return FALSE;
    }

    for (ulIndex = 0; ulIndex < l_ulPluginCount; ++ulIndex)
    {
        pPlugin = &l_pPlugins[ulIndex];

        if (pPlugin->pszName)
        {
            UtFreeMem(pPlugin->pszName);
        }

        if (pPlugin->pDevices)
        {
            UtFreeMem(pPlugin->pDevices);
        }
    }

    if (l_pPlugins)
    {
        UtFreeMem(l_pPlugins);

        l_pPlugins = NULL;
    }

    l_ulPluginCount = 0;
    l_bInitialize = TRUE;

    if (UtUninitHeap() == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL UTPEPDEVICESAPI UtPepDevicesGetDeviceCount(
  PULONG pulDeviceCount)
{
    TPlugin* pPlugin;
    ULONG ulIndex;

    *pulDeviceCount = 0;

    for (ulIndex = 0; ulIndex < l_ulPluginCount; ++ulIndex)
    {
        pPlugin = &l_pPlugins[ulIndex];

        *pulDeviceCount += pPlugin->ulDeviceCount;
    }

    return TRUE;
}

BOOL UTPEPDEVICESAPI UtPepDevicesGetDevice(
  ULONG ulIndex,
  TDevice* pDevice)
{
    TPlugin* pPlugin;
    ULONG ulPluginIndex, ulDeviceIndex;

    ulDeviceIndex = 0;

    for (ulPluginIndex = 0; ulPluginIndex < l_ulPluginCount; ++ulPluginIndex)
    {
        pPlugin = &l_pPlugins[ulPluginIndex];

        if (ulIndex >= ulDeviceIndex &&
            ulIndex < ulDeviceIndex + pPlugin->ulDeviceCount)
        {
            CopyMemory(pDevice, &pPlugin->pDevices[ulIndex - ulDeviceIndex],
                       sizeof(TDevice));

            return TRUE;
        }

        ulDeviceIndex += pPlugin->ulDeviceCount;
    }

    return FALSE;
}

BOOL UTPEPDEVICESAPI UtPepDevicesFindDevice(
  LPCWSTR pszName,
  EDeviceType DeviceType,
  TDevice* pDevice)
{
    ULONG ulDeviceCount, ulIndex;
    TDevice TmpDevice;

    if (FALSE == UtPepDevicesGetDeviceCount(&ulDeviceCount))
    {
        return FALSE;
    }

    for (ulIndex = 0; ulIndex < ulDeviceCount; ++ulIndex)
    {
        if (FALSE == UtPepDevicesGetDevice(ulIndex, &TmpDevice))
        {
            return FALSE;
        }

        if (lstrcmpiW(pszName, TmpDevice.pszName) == 0 &&
            DeviceType == TmpDevice.DeviceType)
        {
            CopyMemory(pDevice, &TmpDevice, sizeof(TDevice));

            return TRUE;
        }
    }

    return FALSE;
}

BOOL UTPEPDEVICESAPI UtPepDevicesGetPluginCount(
  PULONG pulPluginCount)
{
    *pulPluginCount = l_ulPluginCount;

    return TRUE;
}

BOOL UTPEPDEVICESAPI UtPepDevicesGetPluginName(
  ULONG ulIndex,
  LPCWSTR* ppszName)
{
    if (ulIndex >= l_ulPluginCount)
    {
        return FALSE;
    }

    *ppszName = l_pPlugins[ulIndex].pszName;

    return TRUE;
}

BOOL UTPEPDEVICESAPI UtPepDevicesGetPluginVersion(
  ULONG ulIndex,
  PWORD pwProductMajorVersion,
  PWORD pwProductMinorVersion,
  PWORD pwProductBuildVersion,
  PWORD pwProductPrivateVersion)
{
    if (ulIndex >= l_ulPluginCount)
    {
        return FALSE;
    }

    *pwProductMajorVersion   = l_pPlugins[ulIndex].wProductMajorVersion;
    *pwProductMinorVersion   = l_pPlugins[ulIndex].wProductMinorVersion;
    *pwProductBuildVersion   = l_pPlugins[ulIndex].wProductBuildVersion;
    *pwProductPrivateVersion = l_pPlugins[ulIndex].wProductPrivateVersion;

    return TRUE;
}

LPCWSTR UTPEPDEVICESAPI UtPepDevicesGetDeviceTypeName(
  EDeviceType DeviceType)
{
    switch (DeviceType)
    {
        case edtEPROM:
            return L"EPROM";
        case edtEPROMWithIO:
            return L"EPROM With IO";
        case edtPROM:
            return L"PROM";
        case edtSRAM:
            return L"SRAM";
        case edtPAL:
            return L"PAL";
    }

    return L"Unknown";
}

LPCWSTR UTPEPDEVICESAPI UtPepDevicesGetDevicePackageName(
  EDevicePackage DevicePackage)
{
    switch (DevicePackage)
	{
		case edpDIP:
			return L"DIP (Dual-inline Package)";
		case edpPLCC:
			return L"PLCC (Plastic Leaded Chip Carrier)";
	}

	return L"Unknown";
}

LPCWSTR UTPEPDEVICESAPI UtPepDevicesGetDeviceVppName(
  EDeviceVpp DeviceVpp)
{
    switch (DeviceVpp)
    {
        case edv12:
            return L"+12VDC";
        case edv12dot75:
            return L"+12.75VDC";
        case edv21:
            return L"+21VDC";
        case edv25:
            return L"+25VDC";
        case edv26:
            return L"+26VDC";
        case edvNotSupported:
            return L"Not Supported";
    }

    return L"Unknown";
}

LPCWSTR UTPEPDEVICESAPI UtPepDevicesAllocPinDiagram(
  EDevicePackage DevicePackage,
  LPCWSTR* ppszPinNames,
  UINT nPinNamesCount)
{
    switch (DevicePackage)
    {
        case edpDIP:
            return lAllocDipPackagePinDiagram(ppszPinNames, nPinNamesCount);
		case edpPLCC:
			return lAllocPlccPackagePinDiagram(ppszPinNames, nPinNamesCount);
	}

    return NULL;
}

BOOL UTPEPDEVICESAPI UtPepDevicesFreePinDiagram(
  LPCWSTR pszPinDiagram)
{
    UtFreeMem((LPVOID)pszPinDiagram);

    return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
