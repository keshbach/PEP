/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Resource.h"
#include "PepApp.h"

#include "PepAppSplashDialog.h"

#include "UtPepApp.h"

#include <Includes/UtThread.inl>

#include <Includes/UtVersion.h>
#include <Includes/UtMacros.h>

#include <Utils/UtHeapProcess.h>

#include <Hosts/PepAppHost.h>

#pragma region Constants

#define CPepAppRunningMutexName L"PepAppRunningMutex"

#pragma endregion

#pragma region Typedefs

typedef BOOL (PEPAPPHOSTAPI* TPepAppHostInitializeFunc)(VOID);
typedef BOOL (PEPAPPHOSTAPI* TPepAppHostUninitializeFunc)(VOID);
typedef BOOL (PEPAPPHOSTAPI* TPepAppHostExecuteFunc)(_Out_ LPDWORD pdwExitCode);
typedef BOOL (PEPAPPHOSTAPI* TPepAppHostSetPluginPath)(_In_ LPCWSTR pszPluginPath);

typedef VOID (STDAPICALLTYPE *TPathRemoveExtensionW)(_Inout_ LPWSTR pszPath);
typedef BOOL (STDAPICALLTYPE *TPathRemoveFileSpecW)(_Inout_ LPWSTR pszPath);
typedef BOOL (STDAPICALLTYPE *TPathAppendW)(_Inout_ LPWSTR pszPath, _In_ LPCWSTR pszMore);

#pragma endregion

#pragma region Structures

typedef struct tagTPepAppHostData
{
    HMODULE hModule;
    TPepAppHostInitializeFunc pInitialize;
    TPepAppHostUninitializeFunc pUninitialize;
    TPepAppHostExecuteFunc pExecute;
    TPepAppHostSetPluginPath pSetPluginPath;
} TPepAppHostData;

typedef struct tagTSetupData
{
    HINSTANCE hInstance;
    LPCWSTR pszArguments;
    TPepAppHostData AppHostData;
} TSetupData;

#pragma endregion

#pragma region Global Variables

static HANDLE l_hAppRunningMutex = NULL;

#pragma endregion

#pragma region "Local Functions"

static BOOL lIsSupportedOperatingSystem(VOID)
{
    DWORDLONG nConditionMask(0);
    OSVERSIONINFOEX VersionInfoEx;

    VersionInfoEx.dwOSVersionInfoSize = sizeof(VersionInfoEx);
    VersionInfoEx.dwMajorVersion = 6; // set to 6 for vista and above
    VersionInfoEx.dwMinorVersion = 1; // set for Windows 7 and above

    nConditionMask = ::VerSetConditionMask(nConditionMask, VER_MAJORVERSION,
                                           VER_GREATER_EQUAL);
    nConditionMask = ::VerSetConditionMask(nConditionMask, VER_MINORVERSION,
                                           VER_GREATER_EQUAL);

    return ::VerifyVersionInfo(&VersionInfoEx, VER_MAJORVERSION | VER_MINORVERSION,
                               nConditionMask);
}

static BOOL lInitialize(
  _Out_writes_bytes_(sizeof(BOOL)) LPBOOL pbAlreadyRunning)
{
    BOOL bResult = FALSE;

    *pbAlreadyRunning = FALSE;

    // Check if the application is already running

    l_hAppRunningMutex = ::CreateMutex(NULL, FALSE, CPepAppRunningMutexName);

    if (l_hAppRunningMutex)
    {
        if (::GetLastError() == ERROR_ALREADY_EXISTS)
        {
            *pbAlreadyRunning = TRUE;
        }

        bResult = TRUE;
    }
    else
    {
        if (::GetLastError() == ERROR_INVALID_HANDLE)
        {
            // name matches a different object
        }
        else if (::GetLastError() == ERROR_ACCESS_DENIED)
        {
            *pbAlreadyRunning = TRUE;

            bResult = TRUE;
        }
    }

    return bResult;
}

static VOID lUninitialize()
{
    ::CloseHandle(l_hAppRunningMutex);

    l_hAppRunningMutex = NULL;
}

static DWORD WINAPI lRunSetupThreadProc(
  _In_ LPVOID pvParameter)
{
    DWORD dwResult = 0;
    TSetupData* pSetupData = (TSetupData*)pvParameter;
    BOOL bAlreadyRunning;

    if (!lIsSupportedOperatingSystem())
    {
        PepAppSplashDialogDisplayUnsupportedOS();

        return FALSE;
    }

    if (!lInitialize(&bAlreadyRunning))
    {
        PepAppSplashDialogDisplayUnknownError();

        return FALSE;
    }

    if (bAlreadyRunning)
    {
        PepAppSplashDialogDisplayAppAlreadyRunning();

        lUninitialize();

        return FALSE;
    }

    pSetupData->AppHostData.hModule = ::LoadLibrary(L"PepAppHost.dll");

    if (pSetupData->AppHostData.hModule == NULL)
    {
        PepAppSplashDialogDisplayUnknownError();

        lUninitialize();

        return FALSE;
    }

    pSetupData->AppHostData.pInitialize = (TPepAppHostInitializeFunc)::GetProcAddress(pSetupData->AppHostData.hModule, "PepAppHostInitialize");
    pSetupData->AppHostData.pUninitialize = (TPepAppHostUninitializeFunc)::GetProcAddress(pSetupData->AppHostData.hModule, "PepAppHostUninitialize");
    pSetupData->AppHostData.pExecute = (TPepAppHostExecuteFunc)::GetProcAddress(pSetupData->AppHostData.hModule, "PepAppHostExecute");
    pSetupData->AppHostData.pSetPluginPath = (TPepAppHostSetPluginPath)::GetProcAddress(pSetupData->AppHostData.hModule, "PepAppHostSetPluginPath");

    if (pSetupData->AppHostData.pInitialize == NULL ||
        pSetupData->AppHostData.pUninitialize == NULL ||
        pSetupData->AppHostData.pExecute == NULL ||
        pSetupData->AppHostData.pSetPluginPath == NULL ||
        pSetupData->AppHostData.pInitialize() == FALSE)
    {
        ::FreeLibrary(pSetupData->AppHostData.hModule);

        PepAppSplashDialogDisplayUnknownError();

        lUninitialize();

        return FALSE;
    }

    PepAppSplashDialogQuitMessagePump();

    return TRUE;
}

static BOOL lSetPluginPath(
    TPepAppHostData* pPepAppHostData)
{
    WCHAR cPluginPath[MAX_PATH];
    DWORD dwResult;
    HMODULE hModule;
    TPathRemoveExtensionW pPathRemoveExtension;
    TPathRemoveFileSpecW pPathRemoveFileSpec;
    TPathAppendW pPathAppend;

    dwResult = ::GetModuleFileName(::GetModuleHandle(NULL), cPluginPath, MArrayLen(cPluginPath));

    if (dwResult == ERROR_INSUFFICIENT_BUFFER)
    {
        return FALSE;
    }

    hModule = ::LoadLibraryW(L"shlwapi.dll");

    if (hModule == NULL)
    {
        return FALSE;
    }

    pPathRemoveExtension = (TPathRemoveExtensionW)::GetProcAddress(hModule, "PathRemoveExtensionW");
    pPathRemoveFileSpec = (TPathRemoveFileSpecW)::GetProcAddress(hModule, "PathRemoveFileSpecW");
    pPathAppend = (TPathAppendW)::GetProcAddress(hModule, "PathAppendW");

    if (pPathRemoveExtension == NULL || pPathRemoveFileSpec == NULL || pPathAppend == NULL)
    {
        ::FreeLibrary(hModule);

        return FALSE;
    }

    pPathRemoveExtension(cPluginPath);
    pPathRemoveFileSpec(cPluginPath);
    pPathAppend(cPluginPath, L"Plugins");

    ::FreeLibrary(hModule);

    return pPepAppHostData->pSetPluginPath(cPluginPath);
}

#pragma endregion

#pragma region Public Functions

INT PepAppExecute(
  _In_ HINSTANCE hInstance,
  _In_z_ LPCWSTR pszArguments)
{
    HANDLE hThread;
    TSetupData SetupData;
    DWORD dwThreadId, dwExitCode;
    LPCWSTR pszAppTitle, pszMessage;

    if (!PepAppSplashDialogCreate(hInstance))
    {
        pszAppTitle = UtPepAppAllocString(IDS_APPTITLE);
        pszMessage = UtPepAppAllocString(IDS_CANNOTCREATEAPPWINDOW);

        ::MessageBox(NULL, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

        UtPepAppFreeString(pszAppTitle);
        UtPepAppFreeString(pszMessage);

        return 1;
    }

    SetupData.hInstance = hInstance;
    SetupData.pszArguments = pszArguments;

    hThread = ::CreateThread(NULL, 0, lRunSetupThreadProc, &SetupData, 0, &dwThreadId);

    if (hThread == NULL)
    {
        pszAppTitle = UtPepAppAllocString(IDS_APPTITLE);
        pszMessage = UtPepAppAllocString(IDS_CANNOTCREATEINSTALLTHREAD);

        ::MessageBox(NULL, pszMessage, pszAppTitle, MB_OK | MB_ICONINFORMATION);

        UtPepAppFreeString(pszAppTitle);
        UtPepAppFreeString(pszMessage);

        PepAppSplashDialogDestroy();

        return 1;
    }

    UtSetThreadName(dwThreadId, "Setup Thread");

    PepAppSplashDialogMessagePump();

    ::WaitForSingleObject(hThread, INFINITE);

    ::GetExitCodeThread(hThread, &dwExitCode);

    ::CloseHandle(hThread);

    PepAppSplashDialogDestroy();

    if (dwExitCode == FALSE)
    {
        return 1;
    }

    if (FALSE == lSetPluginPath(&SetupData.AppHostData))
    {
        return 1;
    }

    SetupData.AppHostData.pExecute(&dwExitCode);

    SetupData.AppHostData.pUninitialize();

    //::FreeLibrary(SetupData.AppHostData.hModule);

    lUninitialize();

    return dwExitCode;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
