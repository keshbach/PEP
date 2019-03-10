/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "PepSetupResources.h"

#include "UtPepSetup.h"

#include <Includes/UtMacros.h>

#include <Includes/UtExtractResource.inl>

#pragma region Constants

#define CUtListPortsDllFileName L"UtListPorts.dll"
#define CPepSetupMsiFileName L"PepSetup.msi"

#if defined(WIN32)
#define CVCRedistFileName L"vc_redist.x86.exe"
#define CNetFramework4FileName L"dotNetFx40_Full_x86.exe"
#elif defined(WIN64)
#define CVCRedistFileName L"vc_redist.x64.exe"
#define CNetFramework4FileName L"dotNetFx40_Full_x86_x64.exe"
#else
#error Unsupported architecture
#endif

#pragma endregion

#pragma region Structures

typedef struct _TPepSetupResourceData
{
    WCHAR cRootPath[MAX_PATH];
    WCHAR cTempPath[MAX_PATH];
    WCHAR cListPortsFile[MAX_PATH];
    WCHAR cPepSetupMsiFile[MAX_PATH];
    WCHAR cVcRedistFile[MAX_PATH];
    WCHAR cNetFrameworkFile[MAX_PATH];
} TPepSetupResourceData;

#pragma endregion

#pragma region Local Variables

static TPepSetupResourceData l_PepSetupResourceData = {0};

#pragma endregion

#pragma region Local Functions

static VOID lCleanupResources()
{
    ::DeleteFile(l_PepSetupResourceData.cNetFrameworkFile);
    ::DeleteFile(l_PepSetupResourceData.cVcRedistFile);
    ::DeleteFile(l_PepSetupResourceData.cPepSetupMsiFile);
    ::DeleteFile(l_PepSetupResourceData.cListPortsFile);

    ::RemoveDirectory(l_PepSetupResourceData.cTempPath);
    ::RemoveDirectory(l_PepSetupResourceData.cRootPath);
}

#pragma endregion

#pragma region Public Functions

BOOL PepSetupResourcesInitialize(
  _In_ HINSTANCE hInstance)
{
    if (!UtPepSetupGenerateTempDirectoryName(l_PepSetupResourceData.cRootPath, MArrayLen(l_PepSetupResourceData.cRootPath)) ||
        !::CreateDirectory(l_PepSetupResourceData.cRootPath, NULL))
    {
        return FALSE;
    }

    ::PathCombine(l_PepSetupResourceData.cTempPath, l_PepSetupResourceData.cRootPath, L"tmp");

    if (!::CreateDirectory(l_PepSetupResourceData.cTempPath, NULL))
    {
        ::RemoveDirectory(l_PepSetupResourceData.cRootPath);

        return FALSE;
    }

    ::PathCombine(l_PepSetupResourceData.cListPortsFile, l_PepSetupResourceData.cRootPath, CUtListPortsDllFileName);
    ::PathCombine(l_PepSetupResourceData.cPepSetupMsiFile, l_PepSetupResourceData.cRootPath, CPepSetupMsiFileName);
    ::PathCombine(l_PepSetupResourceData.cVcRedistFile, l_PepSetupResourceData.cRootPath, CVCRedistFileName);
    ::PathCombine(l_PepSetupResourceData.cNetFrameworkFile, l_PepSetupResourceData.cRootPath, CNetFramework4FileName);

    if (UtExtractResource(hInstance, RT_BINARY, IDR_UTLISTPORTSDLL, l_PepSetupResourceData.cListPortsFile) &&
        UtExtractResource(hInstance, RT_BINARY, IDR_PEPSETUPMSI, l_PepSetupResourceData.cPepSetupMsiFile) &&
        UtExtractResource(hInstance, RT_BINARY, IDR_VCREDIST, l_PepSetupResourceData.cVcRedistFile) &&
        UtExtractResource(hInstance, RT_BINARY, IDR_NETFRAMEWORK4, l_PepSetupResourceData.cNetFrameworkFile))
    {
        return TRUE;
    }

    lCleanupResources();

    return FALSE;
}

BOOL PepSetupResourcesUninitialize(VOID)
{
    lCleanupResources();

    return TRUE;
}

LPCWSTR PepSetupGetTempPath(VOID)
{
    return l_PepSetupResourceData.cTempPath;
}

LPCWSTR PepSetupGetListPortsFile(VOID)
{
    return l_PepSetupResourceData.cListPortsFile;
}

LPCWSTR PepSetupGetPepSetupMsiFile(VOID)
{
    return l_PepSetupResourceData.cPepSetupMsiFile;
}

LPCWSTR PepSetupGetVcRedistFile(VOID)
{
    return l_PepSetupResourceData.cVcRedistFile;
}

LPCWSTR PepSetupGetNetFrameworkFile(VOID)
{
    return l_PepSetupResourceData.cNetFrameworkFile;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
